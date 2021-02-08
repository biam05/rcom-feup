# RCOM

## TUTORIAL PARA AS PORTAS SERIES VIRTUAIS:

1. abrir um terminal e escrever:

````
sudo socat -d  -d  PTY,link=/dev/ttyS0,mode=777   PTY,link=/dev/ttyS1,mode=777
````

2. abrir mais dois terminais na pasta onde estão os ficheiros

3. compilar:

````
gcc read.c -o read
````


````
gcc write.c -o write
````

ou

````
make clean && make
````

4. num terminal escrever:

````
./read /dev/ttyS0
````

e no outro:

````
./write /dev/ttyS1
````


## TUTORIAL PARA AS PORTAS SERIES:

1. verificar que os cabos estejam conectados e o botão esteja para fora.

2. abrir um terminal em cada computador e escrever:
````
./read /dev/ttyS0
./write /dev/ttyS0
````
