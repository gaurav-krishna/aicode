#include <iostream>
#include <vector>
#include <cstring>
#include "../include/LuaJITWrapper.hpp"

using namespace luajit_embed;

static int cpp_print(lua_State *L) {
  int n = lua_gettop(L);
  for (int i = 1; i <= n; ++i) {
    size_t len = 0;
    const char *s = lua_tolstring(L, i, &len);
    if (!s) {
      // Fallback to tostring conversion
      lua_getglobal(L, "tostring");
      lua_pushvalue(L, i);
      if (lua_pcall(L, 1, 1, 0) == LUA_OK) {
        s = lua_tolstring(L, -1, &len);
        lua_pop(L, 1);
      } else {
        lua_pop(L, 1);
        s = "(non-stringable)";
        len = std::strlen(s);
      }
    }
    std::cout << (i > 1 ? "\t" : "") << std::string(s, len);
  }
  std::cout << std::endl;
  return 0;
}

int main() {
  try {
    LuaJITContext lua(/*openStandardLibraries*/ true,
                      /*enableJIT*/ true,
                      /*jitOptions*/ {"maxtrace=8000", "hotloop=56"});

    lua.disableOsUnsafe();

    lua.registerFunction("cpp_print", cpp_print);

    lua.setGlobal("greeting", LuaValue::String("Hello from C++"));

    lua.runFile("./scripts/example.lua");

    auto results = lua.callGlobal("add", {LuaValue::Integer(40), LuaValue::Integer(2)}, 1);
    if (!results.empty()) {
      const auto &r = results[0];
      if (r.type == LuaValue::Type::Integer)
        std::cout << "add(40,2) = " << r.intValue << std::endl;
      else if (r.type == LuaValue::Type::Number)
        std::cout << "add(40,2) = " << r.numValue << std::endl;
    }

    lua.runString("cpp_print('JIT status:', require('jit').status())", "jit_status");
  } catch (const LuaJITError &e) {
    std::cerr << "LuaJITError: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}