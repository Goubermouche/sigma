workspace "sigma"
    configurations { "Release" }
    startproject "compiler"

    architecture "x64"
    language "C++"
    cppdialect "C++latest"

    externalanglebrackets "On"
    externalwarnings "Off"

    buildoptions { "/wd5054" }

    flags {
        "MultiProcessorCompile"
    }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        warnings "High"

    -- output directories 
    targetdir "output/bin/%{cfg.buildcfg}"
    objdir "output/obj/%{cfg.buildcfg}"

-- utility 
project "utility"
    kind "StaticLib"
    location "source/utility"

    files {
        "source/utility/**.h",
        "source/utility/**.cpp"
    } 

    includedirs {
        "source"
    }

project "tokenizer"
    kind "StaticLib"
    location "source/tokenizer"

    files {
        "source/tokenizer/**.h",
        "source/tokenizer/**.cpp"
    }

    includedirs {
        "source"
    }

    links {
        "utility"
    }

project "abstract_syntax_tree"
    kind "StaticLib"
    location "source/abstract_syntax_tree"

    files {
        "source/abstract_syntax_tree/**.h",
        "source/abstract_syntax_tree/**.cpp"
    } 

    includedirs {
        "source"
    }

    links {
        "utility"
    }

project "parser"
    kind "StaticLib"
    location "source/parser"

    files {
        "source/parser/**.h",
        "source/parser/**.cpp"
    }

    includedirs {
        "source"
    }

    links {
        "tokenizer",
        "abstract_syntax_tree"
    }

project "type_checker"
    kind "StaticLib"
    location "source/type_checker"

    files {
        "source/type_checker/**.h",
        "source/type_checker/**.cpp"
    }

    includedirs {
        "source"
    }

    links {
        "tokenizer",
        "abstract_syntax_tree"
    }

-- IR
project "intermediate_representation"
    kind "StaticLib"
    location "source/intermediate_representation"

    files {
        "source/intermediate_representation/**.h",
        "source/intermediate_representation/**.cpp"
    } 

    includedirs {
        "source"
    }

    links {
        "utility"
    }

project "ir_translator"
    kind "StaticLib"
    location "source/ir_translator"

    files {
        "source/ir_translator/**.h",
        "source/ir_translator/**.cpp"
    }

    includedirs {
        "source"
    }

    links {
        "tokenizer",
        "abstract_syntax_tree",
        "intermediate_representation"
    }

-- compiler
project "compiler"
    kind "ConsoleApp"
    location "source/compiler"

    debugargs { "compile", "-s", "test/main.s", "-o", "test/app.exe" }

    files {
        "source/compiler/**.h",
        "source/compiler/**.cpp"
    }

    includedirs {
        "source"
    }

    links {
        "Version",
        "intermediate_representation",
        "parser",
        "type_checker",
        "ir_translator"
    }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        warnings "High"
        
    filter { "system:linux", "action:gmake" }
        links {
            "dl",     
            "pthread",
            "m",      
            "rt"      
        }

        buildoptions { 
            "-std=c++23"
        }

        linkoptions {
            "-lstdc++"
        }

    filter {}