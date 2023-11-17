# tiny-tools
## ModTool
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
