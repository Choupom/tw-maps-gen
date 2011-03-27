function Intermediate_Output(settings, input)
	return "objs/" .. string.sub(PathBase(input), string.len("src/")+1)
end

settings = NewSettings()
settings.optimize = 1
settings.debug = 0
settings.cc.Output = Intermediate_Output
settings.cc.flags:Add("/EHsc")

zlib = Compile(settings, Collect("src/external/zlib/*.c"))
settings.cc.includes:Add("src/external/zlib")

pnglite = Compile(settings, Collect("src/external/pnglite/*.c"))
settings.cc.includes:Add("src/external/pnglite")

objs = Compile(settings, Collect("src/*.cpp"))

exe = Link(settings, "tw-maps-gen", zlib, pnglite, objs)
