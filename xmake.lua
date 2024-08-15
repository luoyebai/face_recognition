--编译选项设置
----test 模式用于测试
----debug 模式用于调试
----release 模式用于生成
local target_name = "main"

--基础设置
----模式
add_rules("mode.debug", "mode.release")

----设置c,c++版本
set_languages("c99", "c++20")

----设置编译器
set_toolchains("clang")

----debug
if is_mode("debug") then
	set_symbols("debug")
	set_warnings("all", "error")
	add_cxflags("-fsanitize=address")
	add_ldflags("-fsanitize=address")
	set_optimize("none")
end

----release
if is_mode("release") then
	set_symbols("hidden")
	set_optimize("fastest")
	set_strip("all")
end

--第三方包依赖设置
add_requires("gtest", "opencv", "ffmpeg", { configs = { gtk = true } })
add_packages("gtest", "opencv", "ffmpeg")

--具体设置
-- 宏定义
-- 项目位置
add_defines(
	'__PROJECT_DIR__="$(projectdir)"',
	'__OS__="$(host)" ',
	'__DATA_DIR__="$(projectdir)/data/"',
	'__CONFIG_DIR__="$(projectdir)/config/"'
)

----test
if is_mode("test") then
	target("test")
	set_symbols("debug")
	add_files("test/*.cpp")
	set_kind("binary")
	add_syslinks("z", "pthread")
	add_includedirs("/usr/include", "/usr/local/include", "./include")
end

-- xmake自带测试
-- for _, file in ipairs(os.files("test/*.cpp")) do
-- local name = path.basename(file)
-- target(name)
-- set_kind("binary")
-- set_default(false)
-- add_files("test/" .. name .. ".cpp")
-- add_tests("<test name>")
-- end

----项目设置
if is_mode("debug", "release") then
	target(target_name)
	add_files("src/*.cpp")
	set_kind("binary")
	add_syslinks("z", "pthread")
	add_includedirs("/usr/include", "/usr/local/include", "./include")
end

--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--
