param([switch]$Release)
$ErrorActionPreference = 'Stop'
$dir = Join-Path ([Environment]::GetFolderPath('MyDocuments')) 'SWOS'
New-Item -ItemType Directory -Force -Path $dir | Out-Null
$path = Join-Path $dir 'SWOS.ini'
if (Test-Path $path) {
  $backup = Join-Path $dir 'SWOS.ini.prima_di_Expanded_4K.bak'
  if (-not (Test-Path $backup)) { Copy-Item $path $backup }
  $content = [IO.File]::ReadAllText($path)
} else { $content = '' }
$lines = [System.Collections.Generic.List[string]]::new()
$lines.AddRange([string[]]($content -split '\r?\n'))
$videoStart = -1; $videoEnd = $lines.Count
for ($i=0; $i -lt $lines.Count; $i++) {
 if ($lines[$i] -match '^\s*\[video\]\s*$') { $videoStart = $i; continue }
 if ($videoStart -ge 0 -and $lines[$i] -match '^\s*\[') { $videoEnd = $i; break }
}
if ($videoStart -lt 0) { $lines.Add('[video]'); $videoStart=$lines.Count-1; $videoEnd=$lines.Count }
# Development uses a maximized decorated window (SDL handles maximization).
# Public releases always start in desktop fullscreen, never inherit this mode.
$fullscreenMode = if ($Release) { 'desktop' } else { 'off' }
$settings = [ordered]@{ fullscreen=$fullscreenMode; widescreen='stretch'; resolution='auto'; scaling='sharp'; crt_mask='off'; subpixel_scroll='false' }
foreach ($key in $settings.Keys) {
 $found=$false
 for ($i=$videoStart+1; $i -lt $videoEnd; $i++) {
   if ($lines[$i] -match ('^\s*' + [regex]::Escape($key) + '\s*=')) { $lines[$i] = "$key=$($settings[$key])"; $found=$true; break }
 }
 if (-not $found) { $lines.Insert($videoEnd,"$key=$($settings[$key])"); $videoEnd++ }
}
[IO.File]::WriteAllText($path,($lines -join "`r`n"),[Text.UTF8Encoding]::new($false))
Write-Host "Configurazione video ($fullscreenMode) salvata in $path"
