# tiny-tools
###shellcode tools
build.sh => script to create a normal binary and one for debugging
7byteshell => 7 bytes to open a file, as long as the filename is ;
server => can handle get requests for files, probably

### ModTool
Communicate with devices using Modbus TCP/IP  
How to use:  
    $*python ModTool.py ip [options]*  
  **Options:**  
    --function  -f  *int  default=17*  
    --port      -p  *int  default=502*  
    --timeout   -t  *int (miliseconds)  default=500*  
    --slave-id  -s  *int  default=1*  
    --debug     -d  *default=True*  
    --data          *string, max 252 bytes*    
    --tid           *transaction id*    
