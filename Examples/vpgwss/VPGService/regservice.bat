REM This script register service in system
REM Take absolute path to binary
SET bp="%~dp0VPGService.exe"
REM Create service
sc create "VPGService" binpath= %bp% start= auto 
REM Start service
REM TIMEOUT 1
REM Start service
sc start VPGService
REM Timeout
REM TIMEOUT 1