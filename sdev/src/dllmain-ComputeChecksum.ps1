# ComputeChecksum.ps1

# 1. Chiedo la cartella
$path = Read-Host -Prompt "Enter the folder path where your executable resides"

# Verifico che la cartella esista
if (-not (Test-Path $path -PathType Container)) {
    Write-Error "Folder not found: $path"
    exit 1
}

# 2. Chiedo il nome del file
$fileName = Read-Host -Prompt "Enter the executable name (e.g. game.exe)"

# Costruisco il percorso completo
$fullPath = Join-Path -Path $path -ChildPath $fileName

# Verifico che il file esista
if (-not (Test-Path $fullPath -PathType Leaf)) {
    Write-Error "File not found: $fullPath"
    exit 1
}

try {
    # 3. Leggo tutti i byte
    $bytes = [System.IO.File]::ReadAllBytes($fullPath)

    # 4. Calcolo la somma
    $sum = 0
    foreach ($b in $bytes) {
        $sum = ($sum + $b) -band 0xFFFFFFFF
    }

    # 5. Stampo in esadecimale a 8 cifre
    $hex = "{0:X8}" -f $sum
    Write-Host "Checksum (simple sum) for '$fileName' is: $hex"
}
catch {
    Write-Error "Error while reading or processing the file: $_"
    exit 1
}
pause
