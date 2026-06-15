$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Config = Join-Path $Root "ghostcontrol_manba_config.json"
$KillPayload = Join-Path $Root "ELF\GhostControl-Cleanup.elf"
$MainPayload = Join-Path $Root "ELF\GhostControl-ManbaV2-NBJr-USB-Patch.elf"
$DefaultPort = 9021
$DefaultIp = "192.168.1.94"

function Read-LauncherConfig {
    $ip = $DefaultIp
    $port = $DefaultPort
    if (Test-Path -LiteralPath $Config) {
        try {
            $json = Get-Content -LiteralPath $Config -Raw | ConvertFrom-Json
            if ($json.ps5_ip) { $ip = [string]$json.ps5_ip }
            if ($json.ps5_port) { $port = [int]$json.ps5_port }
        } catch {
            Write-Host "Config invalide, elle sera reecrite."
        }
    }

    $answer = Read-Host ("PS5 IP [{0}]" -f $ip)
    if (-not [string]::IsNullOrWhiteSpace($answer)) {
        $ip = $answer.Trim()
    }

    if ([string]::IsNullOrWhiteSpace($ip)) {
        throw "Aucune IP PS5."
    }

    @{ ps5_ip = $ip; ps5_port = $port } | ConvertTo-Json | Set-Content -LiteralPath $Config -Encoding ASCII
    return @{ Ip = $ip; Port = $port }
}

function Send-Payload {
    param(
        [Parameter(Mandatory=$true)] [string]$Payload,
        [Parameter(Mandatory=$true)] [string]$Name,
        [Parameter(Mandatory=$true)] [string]$Ip,
        [Parameter(Mandatory=$true)] [int]$Port
    )

    if (-not (Test-Path -LiteralPath $Payload)) {
        throw ("Payload manquante: {0}" -f $Payload)
    }

    $resolved = Resolve-Path -LiteralPath $Payload
    $bytes = [System.IO.File]::ReadAllBytes($resolved.Path)
    $client = [System.Net.Sockets.TcpClient]::new()
    $client.SendTimeout = 15000
    $client.ReceiveTimeout = 15000
    $client.Connect($Ip, $Port)
    $stream = $client.GetStream()
    $stream.Write($bytes, 0, $bytes.Length)
    $stream.Flush()
    $stream.Dispose()
    $client.Dispose()
    Write-Host ("Envoye {0}: {1} bytes vers {2}:{3}" -f $Name, $bytes.Length, $Ip, $Port)
}

$target = Read-LauncherConfig

Write-Host ""
Write-Host "1/2 GhostControl cleanup..."
Send-Payload -Payload $KillPayload -Name "GhostControl-Cleanup" -Ip $target.Ip -Port $target.Port

Write-Host "Pause 2 secondes..."
Start-Sleep -Seconds 2

Write-Host "2/2 GhostControl Manba V2 NBJr USB Patch..."
Send-Payload -Payload $MainPayload -Name "GhostControl-ManbaV2-NBJr-USB-Patch" -Ip $target.Ip -Port $target.Port

Write-Host ""
Write-Host "Termine."
