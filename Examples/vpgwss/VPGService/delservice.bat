REM
taskkill /IM VPGService.exe /F
REM Stop service first
REM sc stop VPGService
REM Take timeout
REM TIMEOUT 1
REM This script deletes service from system
sc delete VPGService
REM Take timeout
REM TIMEOUT 1