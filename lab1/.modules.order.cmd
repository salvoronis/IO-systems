cmd_/home/salvoroni/ioSystems/lab1/modules.order := {   echo /home/salvoroni/ioSystems/lab1/character_device_driver.ko; :; } | awk '!x[$$0]++' - > /home/salvoroni/ioSystems/lab1/modules.order
