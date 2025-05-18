# Richiesta del percorso completo del file
$FilePath = Read-Host "Inserisci il percorso completo di Game.exe (es: C:\Giochi\Game.exe)"

# Verifica se il file esiste
if (-Not (Test-Path $FilePath)) {
    Write-Error "File non trovato: $FilePath"
    pause
    exit 1
}

# Calcolo SHA-256
$hash = Get-FileHash -Path $FilePath -Algorithm SHA256

# Copia negli appunti
$hash.Hash | Set-Clipboard

# Mostra l'hash
Write-Host "`nSHA-256 hash per '$FilePath':"
Write-Host "$($hash.Hash)`n"
Write-Host "✅ L'hash e'stato copiato negli appunti."

pause
