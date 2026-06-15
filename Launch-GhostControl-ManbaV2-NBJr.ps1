$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Config = Join-Path $Root "ghostcontrol_manba_config.json"
$KillPayload = Join-Path $Root "ELF\GhostControl-Cleanup.elf"
$MainPayload = Join-Path $Root "ELF\GhostControl-ManbaV2-NBJr-USB-Patch.elf"
$DefaultPort = 9021

function Save-LauncherTarget {
    param(
        [Parameter(Mandatory=$true)] [string]$Ip,
        [Parameter(Mandatory=$true)] [int]$Port
    )

    $data = [ordered]@{
        ps5_ip = $Ip
        ps5_port = $Port
    }

    $data | ConvertTo-Json | Set-Content -LiteralPath $Config -Encoding UTF8
    Write-Host ("Config locale sauvegardee: {0}" -f $Config)
}

function Read-LauncherTarget {
    $ip = ""
    $port = $DefaultPort

    if (Test-Path -LiteralPath $Config) {
        try {
            $json = Get-Content -LiteralPath $Config -Raw | ConvertFrom-Json
            if ($json.ps5_ip) { $ip = [string]$json.ps5_ip }
            if ($json.ps5_port) { $port = [int]$json.ps5_port }
        } catch {
            Write-Host "Config locale invalide, ignoree."
            $ip = ""
            $port = $DefaultPort
        }
    }

    if ([string]::IsNullOrWhiteSpace($ip)) {
        $answer = Read-Host "PS5 IP"
    } else {
        $answer = Read-Host ("PS5 IP [{0}]" -f $ip)
    }

    if (-not [string]::IsNullOrWhiteSpace($answer)) {
        $ip = $answer.Trim()
    }

    if ([string]::IsNullOrWhiteSpace($ip)) {
        throw "Aucune IP PS5."
    }

    $portAnswer = Read-Host ("Port payload [{0}]" -f $port)
    if (-not [string]::IsNullOrWhiteSpace($portAnswer)) {
        $port = [int]$portAnswer.Trim()
    }

    Save-LauncherTarget -Ip $ip -Port $port

    return @{ Ip = $ip; Port = $port }
}

function Read-LaunchMode {
    Write-Host ""
    Write-Host "Choisir l'action:"
    Write-Host "  1 - KILL puis ELF"
    Write-Host "  2 - ELF seulement"
    Write-Host "  3 - KILL seulement"
    Write-Host ""

    $answer = Read-Host "Choix [2]"
    if ([string]::IsNullOrWhiteSpace($answer)) {
        $answer = "2"
    }

    switch ($answer.Trim()) {
        "1" { return "KillThenElf" }
        "2" { return "ElfOnly" }
        "3" { return "KillOnly" }
        default { throw "Choix invalide. Utilise 1, 2 ou 3." }
    }
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

$target = Read-LauncherTarget
$mode = Read-LaunchMode

Write-Host ""

switch ($mode) {
    "KillThenElf" {
        Write-Host "1/2 KILL PAYLOAD PID..."
        Send-Payload -Payload $KillPayload -Name "KILL PAYLOAD PID" -Ip $target.Ip -Port $target.Port

        Write-Host "Pause 2 secondes..."
        Start-Sleep -Seconds 2

        Write-Host "2/2 GhostControl Manba V2 NBJr USB Patch..."
        Send-Payload -Payload $MainPayload -Name "GhostControl-ManbaV2-NBJr-USB-Patch" -Ip $target.Ip -Port $target.Port
    }

    "ElfOnly" {
        Write-Host "ELF seulement..."
        Send-Payload -Payload $MainPayload -Name "GhostControl-ManbaV2-NBJr-USB-Patch" -Ip $target.Ip -Port $target.Port
    }

    "KillOnly" {
        Write-Host "KILL PAYLOAD PID..."
        Send-Payload -Payload $KillPayload -Name "KILL PAYLOAD PID" -Ip $target.Ip -Port $target.Port
    }
}

Write-Host ""
Write-Host "Termine."
