function split(inputstr, sep)
    if sep == nil then
        sep = "%s" -- default to splitting on spaces
    end
    local t = {}
    for str in string.gmatch(inputstr, "([^" .. sep .. "]+)") do
        table.insert(t, str)
    end
    return t
end

-- get the LLVM installation path
local llvm_root = os.getenv("LLVM_ROOT")
local script_root = path.getabsolute(".")
print("LLVM_ROOT: " .. llvm_root)

workspace "sigma"
    configurations { "Release" }
    startproject "compiler"

    architecture "x64"
    language "C++"
    cppdialect "C++latest"

    externalanglebrackets "On"
    externalwarnings "Off"

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
        "source",
        path.join(llvm_root, "include")
    }

    libdirs {   
        path.join(llvm_root, "lib"),
        path.join(llvm_root, "bin"),
        path.join(llvm_root, "../llvm-build/lib"),
        path.join(llvm_root, "Release/lib")
    }

    local llvm_libs = os.outputof("llvm-config --libs support core")
    local llvm_libs_table = split(llvm_libs)

    links {
        llvm_libs_table
    }

-- lexer
project "lexer"
    kind "StaticLib"
    location "source/lexer"

    files {
        "source/lexer/**.h",
        "source/lexer/**.cpp"
    } 

    includedirs {
        "source",
        path.join(llvm_root, "include")
    }

    links {
        "utility"
    }

-- abstract_syntax_tree
project "abstract_syntax_tree"
    kind "StaticLib"
    location "source/abstract_syntax_tree"

    files {
        "source/abstract_syntax_tree/**.h",
        "source/abstract_syntax_tree/**.cpp"
    } 

    includedirs {
        "source",
        path.join(llvm_root, "include")
    }

    links {
        "utility"
    }


-- parser
project "parser"
    kind "StaticLib"
    location "source/parser"

    files {
        "source/parser/**.h",
        "source/parser/**.cpp"
    } 

    includedirs {
        "source",
        path.join(llvm_root, "include")
    }

    links {
        "abstract_syntax_tree",
        "utility"
    }

    
-- code_generator
project "code_generator"
    kind "StaticLib"
    location "source/code_generator"

    files {
        "source/code_generator/**.h",
        "source/code_generator/**.cpp"
    } 

    includedirs {
        "source"
    }

    links {
        "utility"
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
        "source",
        path.join(llvm_root, "include")
    }

    libdirs {   
        path.join(llvm_root, "lib"),
        path.join(llvm_root, "bin"),
        path.join(llvm_root, "../llvm-build/lib"),
        path.join(llvm_root, "Release/lib")
    }

    local llvm_libs = os.outputof("llvm-config --libs all")
    local llvm_libs_table = split(llvm_libs)

    local clang_libs = { 
        "clangAPINotes",
        "clangARCMigrate",
        "clangAST",
        "clangASTMatchers",
        "clangAnalysis",
        "clangAnalysisFlowSensitive",
        "clangAnalysisFlowSensitiveModels",
        "clangBasic",
        "clangCodeGen",
        "clangCrossTU",
        "clangDependencyScanning",
        "clangDirectoryWatcher",
        "clangDriver",
        "clangDynamicASTMatchers",
        "clangEdit",
        "clangExtractAPI",
        "clangFormat",
        "clangFrontend",
        "clangFrontendTool",
        "clangHandleCXX",
        "clangHandleLLVM",
        "clangIndex",
        "clangIndexSerialization",
        "clangInterpreter",
        "clangLex",
        "clangParse",
        "clangRewrite",
        "clangRewriteFrontend",
        "clangSema",
        "clangSerialization",
        "clangStaticAnalyzerCheckers",
        "clangStaticAnalyzerCore",
        "clangStaticAnalyzerFrontend",
        "clangSupport",
        "clangTooling",
        "clangToolingASTDiff",
        "clangToolingCore",
        "clangToolingInclusions",
        "clangToolingInclusionsStdlib",
        "clangToolingRefactoring",
        "clangToolingSyntax",
        "clangTransformer"
    } 
    
    links {
        llvm_libs_table,
        clang_libs,
        "Version",
        "lexer",
        "parser",
        "code_generator"
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