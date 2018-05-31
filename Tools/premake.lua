-- premake5.lua
workspace "MitchEngine"
   configurations { "Debug", "Release" }
   location "../"
   includedirs {
   "../MitchEngine/Source",
   "../ThirdParty/GLM/glm",
   "../ThirdParty/GLFW/include",
   "../ThirdParty/SOIL/include",
   "../ThirdParty/GLEW/include",
   "../ThirdParty/Boost",
   "../ThirdParty/JsonCPP/include"
   }
   libdirs {
	"../ThirdParty/SOIL/libs",
	"../ThirdParty/GLEW/lib/%{cfg.buildcfg}/Win32",
	"../ThirdParty/GLFW/lib-vc2015",
	"../ThirdParty/JsonCPP/**/%{cfg.buildcfg}"
   }
   
   links {
   	   "SOIL",
	   "glfw3",
	   "opengl32",
	   "lib_json"
   }

   filter "configurations:Debug"
	   links {
		   "glew32d"
	   }
   
   filter "configurations:Release"
   links {
	   "glew32"
   }
   
project "MitchEngine"
   kind "StaticLib"
   language "C++"
   targetdir "../Build/%{cfg.buildcfg}"
   location "../MitchEngine"
	files { 
			"../MitchEngine/Source/**.h", 
			"../MitchEngine/Source/**.cpp", 
			"../MitchEngine/Source/**.txt" 
	}

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

project "MitchGame"
   kind "ConsoleApp"
   language "C++"
   targetdir "../Build/%{cfg.buildcfg}"
   location "../MitchGame"
   links "MitchEngine"
   
	files { 
			"../MitchGame/Source/**.h", 
			"../MitchGame/Source/**.cpp", 
			"../MitchGame/Source/**.txt" 
	}

   files { "**.h", "**.c" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"