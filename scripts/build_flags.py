Import("env")

cpp_standard = env.GetProjectOption("cppstd");
c_standard = env.GetProjectOption("cstd");

# General options that are passed to the C and C++ compilers
# env.Append(CCFLAGS=["flag1", "flag2"])

# General options that are passed to the C compiler (C only; not C++).
env.Append(CFLAGS=[
  "--language=c",
  "-std=" + c_standard,
  "-flto=auto",
  "-fuse-linker-plugin",
])

# General options that are passed to the C++ compiler
env.Append(CXXFLAGS=[
  "--language=c++",
  "-std=" + cpp_standard,
  "-flto=auto",
  "-fuse-linker-plugin",
  "-fpermissive",
  "-fno-threadsafe-statics",
])

env.Append(LINKFLAGS=[
   "-Wall",
   "-Wextra",
   "-Os",
   "-g",
   "-flto=auto",
   "-fuse-linker-plugin",
   "-Wl,--gc-sections",
   "-Wl,--relax",
])

print("C flags: " + env['CFLAGS'])
print("CXX flags: " + env['CXXFLAGS'])
print("Linker flags: " + env['LINKFLAGS'])
print("\n")
