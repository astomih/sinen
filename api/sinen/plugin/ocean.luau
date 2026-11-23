--!strict

export type WaveSystem = {
	spectrum: ("phillips" | "jonswap")?,
	significantWaveHeight: number?,
	windSpeed: number?,
	direction: number?,
	peakPeriod: number?,
	peakEnhancement: number?,
	spread: number?,
	shortWaveDamping: number?,
}
export type Settings = {
	resolution: number?, patchSize: number?, depth: number?, gravity: number?,
	choppiness: number?, foamThreshold: number?, foamGain: number?, seed: number?,
	wind: WaveSystem?, swell: WaveSystem?,
}
export type SurfaceSample = {
	height: number, displacementX: number, displacementZ: number,
	normalX: number, normalY: number, normalZ: number, foam: number,
}
export type Ocean = {
	update: (self: Ocean, timeSeconds: number) -> nil,
	sample: (self: Ocean, x: number, z: number) -> SurfaceSample,
	createMeshBuffers: (self: Ocean) -> (buffer, buffer),
	writeVertices: (self: Ocean, vertices: buffer) -> nil,
	info: (self: Ocean) -> { resolution: number, patchSize: number, vertexCount: number, indexCount: number },
}

return {} :: { new: (settings: Settings?) -> Ocean }
