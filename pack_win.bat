@echo off

regsvr32 /u /s "%USERPROFILE%\AppData\Local\Google\Chrome\User Data\Default\Extensions\poeehflodfpibolpjekinkchicnigkfe\0.1_0\npSockIt.dll"

rm "%USERPROFILE%\AppData\Local\Google\Chrome\User Data\Default\Extensions\poeehflodfpibolpjekinkchicnigkfe\0.1_0\npSockIt.dll"

chrome.exe --pack-extension=%CD%\bin\win\win --no-message-box

move /Y bin\win\win.crx bin\win\sockit_win.crx

@echo on
