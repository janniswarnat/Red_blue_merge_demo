### To forward X11 from inside a docker container to a host running macOS
### From: https://gist.github.com/cschiewek/246a244ba23da8b9f0e7b11a68bf3285

1. Install XQuartz: https://www.xquartz.org/
2. Launch XQuartz.  Under the XQuartz menu, select Preferences
3. Go to the security tab and ensure "Allow connections from network clients" is checked.
4. Run `xhost + ${hostname}` to allow connections to the macOS host. Also try `xhost + localhost`.
5. Restart XQuartz.
6. Add the following to your docker-compose:
```
    environment:
      - DISPLAY=host.docker.internal:0
 ```
