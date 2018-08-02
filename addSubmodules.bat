for /f %%f in ('dir /b blocks') do (
	blocks\%%f\addmodule.bat
)
git submodule init
git submodule update

pause
