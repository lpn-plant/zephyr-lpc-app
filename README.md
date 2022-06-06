# zephyr-lpc-app

* Create workspace

```bash
west init -m https://github.com/lpn-plant/zephyr-lpc-app/ --mr main zephyr-lpc-workspace
```

* Clone repositories:

```bash
cd zephyr-lpc-workspace
west udpate
```

* Build project:

```bash
west build -b esp32 -s zephyr-lpc-app/app/
```
