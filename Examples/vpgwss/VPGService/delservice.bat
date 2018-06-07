REM Stop service first
sc stop VPGService
REM Take timeout
TIMEOUT 5
REM This script deletes service from system
sc delete VPGService
REM Take timeout
TIMEOUT 1