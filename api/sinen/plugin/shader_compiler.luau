--!strict

-- Luau definition file for the Slang shader compiler plugin.

local sn = require("@sinen")

export type CompiledShader = {
	code: string,
	numSamplers: number,
	numStorageBuffers: number,
	numStorageTextures: number,
	numUniformBuffers: number,
	uniformBuffers: { sn.ShaderResourceBinding },
	textures: { sn.ShaderResourceBinding },
}

export type ShaderCompiler = {
	compile: (path: string, shaderStage: sn.ShaderStage, shaderFormat: sn.ShaderFormat) -> CompiledShader,
	compileSource: (
		moduleName: string,
		modulePath: string,
		source: string,
		shaderStage: sn.ShaderStage,
		shaderFormat: sn.ShaderFormat
	) -> CompiledShader,
}

return {} :: ShaderCompiler
