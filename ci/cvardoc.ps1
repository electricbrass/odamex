Set-PSDebug -Trace 1

Set-Location "build"
New-Item -Name "wiki-cvardoc" -ItemType "directory" | Out-Null

# Copy all built files into cvardoc directory
Copy-Item -Path `
    ".\client\RelWithDebInfo\odamex.exe", `
    ".\client\RelWithDebInfo\odamex.pdb", `
    ".\client\RelWithDebInfo\*.dll", `
    ".\server\RelWithDebInfo\odasrv.exe", `
    ".\server\RelWithDebInfo\odasrv.pdb", `
    ".\odalaunch\RelWithDebInfo\odalaunch.exe", `
    ".\odalaunch\RelWithDebInfo\odalaunch.pdb", `
    ".\odalaunch\RelWithDebInfo\*.dll", `
    ".\wad\odamex.wad", `
    "..\ci\cvardoc.cfg", `
    "C:\Windows\System32\msvcp140.dll", `
    "C:\Windows\System32\vcruntime140.dll", `
    "C:\Windows\System32\vcruntime140_1.dll" `
    -Destination "wiki-cvardoc"

Set-Location "wiki-cvardoc"

.\odamex.exe -iwad fake.wad +exec "cvardoc.cfg"
.\odasrv.exe +exec "cvardoc.cfg"

Set-Location ..
Set-Location ..