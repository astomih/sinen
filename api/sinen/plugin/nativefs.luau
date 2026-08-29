--!strict

-- Luau definition file for the unrestricted nativefs plugin.

export type NativeFs = {
	read: (path: string) -> (string?, string?),
	readText: (path: string) -> (string?, string?),
	exists: (path: string) -> boolean,
	enumerateDirectory: (path: string) -> ({ string }?, string?),
	getCurrentDirectory: () -> (string?, string?),
	getAbsolutePath: (path: string) -> (string?, string?),
}

return {} :: NativeFs
