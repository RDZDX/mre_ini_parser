"C:\Program Files\MRE_SDK\tools\DllPackage.exe" "D:\MyGitHub\mre_ini_parser\mre_ini_parser.vcproj"
if %errorlevel% == 0 (
 echo postbuild OK.
  copy mre_ini_parser.vpp ..\..\..\MoDIS_VC9\WIN32FS\DRIVE_E\mre_ini_parser.vpp /y
exit 0
)else (
echo postbuild error
  echo error code: %errorlevel%
  exit 1
)

