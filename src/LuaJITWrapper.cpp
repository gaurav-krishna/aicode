#include "../include/LuaJITWrapper.hpp"

#include <sstream>
#include <cstring>
#include <cmath>
#include <climits>

namespace luajit_embed {

namespace {

int pushTracebackFunction(lua_State *L) {
  // LuaJIT (Lua 5.1 API): lua_getglobal returns void, so inspect the stack
  lua_getglobal(L, "debug"); // pushes debug
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    lua_pushcfunction(L, [](lua_State *L_) -> int {
      const char *msg = lua_tostring(L_, 1);
      lua_pushfstring(L_, "[no debug.traceback available] %s", msg ? msg : "(nil)");
      return 1;
    });
    return lua_gettop(L);
  }
  lua_getfield(L, -1, "traceback");
  lua_remove(L, -2); // remove debug table
  return lua_gettop(L);
}

std::string typeToString(int t) {
  switch (t) {
  case LUA_TNIL: return "nil";
  case LUA_TBOOLEAN: return "boolean";
  case LUA_TLIGHTUSERDATA: return "lightuserdata";
  case LUA_TNUMBER: return "number";
  case LUA_TSTRING: return "string";
  case LUA_TTABLE: return "table";
  case LUA_TFUNCTION: return "function";
  case LUA_TUSERDATA: return "userdata";
  case LUA_TTHREAD: return "thread";
  default: return "unknown";
  }
}

} // namespace

LuaJITContext::LuaJITContext(bool openStandardLibraries,
                             bool enableJIT,
                             const std::vector<std::string> &jitOptions) {
  luaState = luaL_newstate();
  if (!luaState) {
    throw LuaJITError("Failed to create Lua state");
  }

  if (openStandardLibraries) {
    luaL_openlibs(luaState);
  }

  enableJITWithOptions(enableJIT, jitOptions);
}

LuaJITContext::~LuaJITContext() {
  if (luaState) {
    lua_close(luaState);
    luaState = nullptr;
  }
}

void LuaJITContext::enableJITWithOptions(bool enableJIT,
                                         const std::vector<std::string> &jitOptions) {
  // require 'jit'
  lua_getglobal(luaState, "require");
  lua_pushstring(luaState, "jit");
  if (lua_pcall(luaState, 1, 1, 0) != LUA_OK) {
    std::string err = lua_tostring(luaState, -1);
    lua_pop(luaState, 1);
    throw LuaJITError(std::string("Failed to require 'jit': ") + err);
  }

  // Stack: [jit_table]
  if (enableJIT) {
    lua_getfield(luaState, -1, "on");
    if (lua_pcall(luaState, 0, 0, 0) != LUA_OK) {
      std::string err = lua_tostring(luaState, -1);
      lua_pop(luaState, 2); // error + jit table
      throw LuaJITError(std::string("Failed to enable JIT: ") + err);
    }
  } else {
    lua_getfield(luaState, -1, "off");
    if (lua_pcall(luaState, 0, 0, 0) != LUA_OK) {
      std::string err = lua_tostring(luaState, -1);
      lua_pop(luaState, 2);
      throw LuaJITError(std::string("Failed to disable JIT: ") + err);
    }
  }

  // Apply jit.opt options if provided
  if (!jitOptions.empty()) {
    // require 'jit.opt'
    lua_getglobal(luaState, "require");
    lua_pushstring(luaState, "jit.opt");
    if (lua_pcall(luaState, 1, 1, 0) != LUA_OK) {
      std::string err = lua_tostring(luaState, -1);
      lua_pop(luaState, 2); // error + jit table
      throw LuaJITError(std::string("Failed to require 'jit.opt': ") + err);
    }

    // Stack: [jit_table][jitopt_table]
    lua_getfield(luaState, -1, "start");
    for (const auto &opt : jitOptions) {
      lua_pushstring(luaState, opt.c_str());
    }
    if (lua_pcall(luaState, static_cast<int>(jitOptions.size()), 0, 0) != LUA_OK) {
      std::string err = lua_tostring(luaState, -1);
      lua_pop(luaState, 3); // error + jitopt + jit
      throw LuaJITError(std::string("Failed to set jit options: ") + err);
    }

    lua_pop(luaState, 1); // pop jitopt table
  }

  lua_pop(luaState, 1); // pop jit table
}

void LuaJITContext::runString(const std::string &code, const std::string &chunkName) {
  int loadStatus = luaL_loadbuffer(luaState, code.c_str(), code.size(), chunkName.c_str());
  if (loadStatus != LUA_OK) {
    std::string msg = lua_tostring(luaState, -1);
    lua_pop(luaState, 1);
    throw LuaJITError(std::string("Failed to load Lua chunk: ") + msg);
  }
  pcallWithTraceback(0, 0, "running chunk");
}

void LuaJITContext::runFile(const std::string &filePath) {
  int loadStatus = luaL_loadfile(luaState, filePath.c_str());
  if (loadStatus != LUA_OK) {
    std::string msg = lua_tostring(luaState, -1);
    lua_pop(luaState, 1);
    throw LuaJITError(std::string("Failed to load file '") + filePath + "': " + msg);
  }
  pcallWithTraceback(0, 0, std::string("running file: ") + filePath);
}

void LuaJITContext::setGlobal(const std::string &name, const LuaValue &value) {
  pushValue(value);
  lua_setglobal(luaState, name.c_str());
}

std::optional<LuaValue> LuaJITContext::getGlobal(const std::string &name) const {
  lua_getglobal(luaState, name.c_str());
  int t = lua_type(luaState, -1);
  if (t == LUA_TNIL) {
    lua_pop(luaState, 1);
    return std::nullopt;
  }
  LuaValue v = readValueFromStack(-1);
  lua_pop(luaState, 1);
  return v;
}

void LuaJITContext::registerFunction(const std::string &name, CFunction function) {
  lua_pushcfunction(luaState, function);
  lua_setglobal(luaState, name.c_str());
}

std::vector<LuaValue> LuaJITContext::callGlobal(const std::string &functionName,
                                                const std::vector<LuaValue> &args,
                                                int expectedReturns) {
  lua_getglobal(luaState, functionName.c_str());
  if (!lua_isfunction(luaState, -1)) {
    lua_pop(luaState, 1);
    throw LuaJITError("Global '" + functionName + "' is not a function");
  }

  for (const auto &arg : args) {
    pushValue(arg);
  }

  pcallWithTraceback(static_cast<int>(args.size()), expectedReturns, std::string("calling function '") + functionName + "'");

  std::vector<LuaValue> results;
  if (expectedReturns > 0) {
    results.reserve(expectedReturns);
    int base = lua_gettop(luaState) - expectedReturns + 1;
    for (int i = 0; i < expectedReturns; ++i) {
      results.emplace_back(readValueFromStack(base + i));
    }
    lua_pop(luaState, expectedReturns);
  }

  return results;
}

void LuaJITContext::disableOsUnsafe() {
  lua_getglobal(luaState, "os");
  if (lua_istable(luaState, -1)) {
    lua_pushnil(luaState);
    lua_setfield(luaState, -2, "execute");

    lua_pushnil(luaState);
    lua_setfield(luaState, -2, "remove");

    lua_pushnil(luaState);
    lua_setfield(luaState, -2, "rename");

    lua_pushnil(luaState);
    lua_setfield(luaState, -2, "tmpname");
  }
  lua_pop(luaState, 1);
}

void LuaJITContext::pcallWithTraceback(int nargs, int nresults, const std::string &contextMessage) {
  // Push traceback function and insert it below the function and its args
  pushTracebackFunction(luaState); // [ ... func args ... err ]
  int errfuncIndex = lua_gettop(luaState) - nargs - 1; // index where func currently is
  lua_insert(luaState, errfuncIndex);                  // [ ... err func args ... ]

  if (lua_pcall(luaState, nargs, nresults, errfuncIndex) != LUA_OK) {
    throwLuaErrorFromTop(contextMessage);
  }

  // Remove traceback function
  lua_remove(luaState, errfuncIndex);
}

[[noreturn]] void LuaJITContext::throwLuaErrorFromTop(const std::string &contextMessage) const {
  const char *msg = lua_tostring(luaState, -1);
  std::string fullMsg = contextMessage + ": " + (msg ? msg : "unknown error");
  lua_pop(luaState, 1);
  throw LuaJITError(fullMsg);
}

void LuaJITContext::pushValue(const LuaValue &value) {
  switch (value.type) {
  case LuaValue::Type::Nil:
    lua_pushnil(luaState);
    break;
  case LuaValue::Type::Boolean:
    lua_pushboolean(luaState, value.boolValue);
    break;
  case LuaValue::Type::Integer:
    lua_pushinteger(luaState, static_cast<lua_Integer>(value.intValue));
    break;
  case LuaValue::Type::Number:
    lua_pushnumber(luaState, static_cast<lua_Number>(value.numValue));
    break;
  case LuaValue::Type::String:
    lua_pushlstring(luaState, value.strValue.c_str(), value.strValue.size());
    break;
  }
}

LuaValue LuaJITContext::readValueFromStack(int index) const {
  int t = lua_type(luaState, index);
  switch (t) {
  case LUA_TBOOLEAN:
    return LuaValue::Boolean(lua_toboolean(luaState, index) != 0);
  case LUA_TNUMBER: {
    double v = static_cast<double>(lua_tonumber(luaState, index));
    if (v >= static_cast<double>(LLONG_MIN) && v <= static_cast<double>(LLONG_MAX)) {
      double ipart = std::floor(v);
      if (ipart == v) {
        return LuaValue::Integer(static_cast<long long>(v));
      }
    }
    return LuaValue::Number(v);
  }
  case LUA_TSTRING: {
    size_t len = 0;
    const char *s = lua_tolstring(luaState, index, &len);
    return LuaValue::String(std::string(s, len));
  }
  case LUA_TNIL:
    return LuaValue::Nil();
  default: {
    std::ostringstream oss;
    oss << "Unsupported Lua type: " << typeToString(t);
    throw LuaJITError(oss.str());
  }
  }
}

} // namespace luajit_embed