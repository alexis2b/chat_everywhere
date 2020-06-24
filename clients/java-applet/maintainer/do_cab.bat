@ECHO OFF
SET PACKAGE="ChatEverywhere"
SET CABDIR="ChatEverywhere-signed-ie"
SET PRIOR="low"
SET CERT_FILE="ms_alexis.spc"
SET KEY_FILE="ms_alexisb.pvk"

SET PATH=%PATH%;c:\progra~1\micros~1.0\bin

ECHO *********** About to create .cab archive using cabarc ***********
cabarc -r -p -P %CABDIR%\ N %CABDIR%.cab %CABDIR%\*.*

ECHO *********** About to sign archive using signcode ***********
signcode -j javasign.dll -jp %PRIOR% -spc %CERT_FILE% -v %KEY_FILE% -n %PACKAGE% %CABDIR%.cab

ECHO *********** About to timestamp .cab archive using signcode ***********
signcode -x -t http://timestamp.verisign.com/scripts/timstamp.dll -tr 5 %CABDIR%.cab

REM Punt the various environment variables
SET PACKAGE=
SET CABDIR=
SET PRIOR=
SET CERT_FILE=
SET KEY_FILE=

ECHO *********** Done timestamping .cab archive ***********

