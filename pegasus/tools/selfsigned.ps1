# Make a self-signed certificate for 127.0.0.1 and localhost, for the loopback tests of worlds over https (a TEST thing: pegasus
# trusts it only when PEGASUS_TEST_TRUST_CA names the PEM file). Run: pwsh tools\selfsigned.ps1 OUTDIR
# Writes OUTDIR\cert.pem and OUTDIR\key.pem (and OUTDIR\other.pem: a second certificate, for the test of a certificate that is
# not the one trusted). Needs openssl on PATH (Git for Windows has one).
param([Parameter(Mandatory)][string]$OutDir)
New-Item -ItemType Directory -Force $OutDir | Out-Null
$ext = "subjectAltName=DNS:localhost,IP:127.0.0.1"
foreach ($pair in @(@('cert', 'key'), @('other', 'otherkey'))) {
  & openssl req -x509 -newkey rsa:2048 -nodes -days 30 -subj "/CN=localhost" -addext $ext `
    -keyout (Join-Path $OutDir "$($pair[1]).pem") -out (Join-Path $OutDir "$($pair[0]).pem") 2>&1 | Out-Null
}
Get-ChildItem $OutDir | Select-Object Name
