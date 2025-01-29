workspace("borepack-bot")
configurations({ "Debug", "Release" })

project("borepack-bot")
kind("ConsoleApp")
language("C++")
targetdir("bin/")

links({ "dpp" })

files({
	"src/**.h",
	"src/**.cpp",
})

filter("configurations:Debug")
defines({ "DEBUG" })
symbols("On")

filter("configurations:Release")
defines({ "NDEBUG" })
optimize("On")
