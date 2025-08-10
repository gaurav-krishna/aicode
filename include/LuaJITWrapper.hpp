#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <functional>
#include <optional>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace luajit_embed {

class LuaJITError : public std::runtime_error {
public:
  explicit LuaJITError(const std::string &message) : std::runtime_error(message) {}
};

struct LuaValue {
  enum class Type { Nil, Boolean, Integer, Number, String };

  Type type{Type::Nil};
  bool boolValue{false};
  long long intValue{0};
  double numValue{0.0};
  std::string strValue;

  static LuaValue Nil() { return LuaValue{}; }
  static LuaValue Boolean(bool v) {
    LuaValue val; val.type = Type::Boolean; val.boolValue = v; return val;
  }
  static LuaValue Integer(long long v) {
    LuaValue val; val.type = Type::Integer; val.intValue = v; return val;
  }
  static LuaValue Number(double v) {
    LuaValue val; val.type = Type::Number; val.numValue = v; return val;
  }
  static LuaValue String(std::string v) {
    LuaValue val; val.type = Type::String; val.strValue = std::move(v); return val;
  }
};

class LuaJITContext {
public:
  using CFunction = lua_CFunction;

  explicit LuaJITContext(bool openStandardLibraries = true,
                         bool enableJIT = true,
                         const std::vector<std::string> &jitOptions = {});
  ~LuaJITContext();

  LuaJITContext(const LuaJITContext &) = delete;
  LuaJITContext &operator=(const LuaJITContext &) = delete;

  void runString(const std::string &code, const std::string &chunkName = "chunk");
  void runFile(const std::string &filePath);

  void setGlobal(const std::string &name, const LuaValue &value);
  std::optional<LuaValue> getGlobal(const std::string &name) const;

  void registerFunction(const std::string &name, CFunction function);

  std::vector<LuaValue> callGlobal(const std::string &functionName,
                                   const std::vector<LuaValue> &args = {},
                                   int expectedReturns = 0);

  lua_State *state() { return luaState; }

  // Optional: disable potentially unsafe standard libraries (basic sandboxing)
  void disableOsUnsafe();

private:
  lua_State *luaState{nullptr};

  [[noreturn]] void throwLuaErrorFromTop(const std::string &contextMessage) const;
  void pushValue(const LuaValue &value);
  LuaValue readValueFromStack(int index) const;
  void pcallWithTraceback(int nargs, int nresults, const std::string &contextMessage);
  void enableJITWithOptions(bool enableJIT, const std::vector<std::string> &jitOptions);
};

} // namespace luajit_embed