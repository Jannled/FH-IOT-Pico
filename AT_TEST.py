"""
Raspberry Pi Pico (MicroPython) exercise:
work with SIM7080G Cat-M/NBIoT Module
"""
import machine
import utime

# using pin defined
pwr_en = 14  # pin to control the power of the module

# uart setting
uart_port = 0
uart_baute = 115200
Pico_SIM7080G = machine.UART(uart_port, uart_baute)

# LED indicator on Raspberry Pi Pico
led_pin = 25  # onboard led
led_onboard = machine.Pin(led_pin, machine.Pin.OUT)


def led_blink():
    for i in range(1, 3):
        led_onboard.value(1)
        utime.sleep(1)
        led_onboard.value(0)
        utime.sleep(1)
    led_onboard.value(0)


# power on/off the module
def module_power():
    pwr_key = machine.Pin(pwr_en, machine.Pin.OUT)
    pwr_key.value(1)
    utime.sleep(2)
    pwr_key.value(0)


# Send AT command
def send_at(cmd, back, timeout=1000):
    rec_buff = b''
    Pico_SIM7080G.write((cmd + '\r\n').encode())
    prvmills = utime.ticks_ms()
    while (utime.ticks_ms() - prvmills) < timeout:
        if Pico_SIM7080G.any():
            rec_buff = b"".join([rec_buff, Pico_SIM7080G.read(1)])
    if rec_buff != '':
        if back not in rec_buff.decode():
            print(cmd + ' back:\t' + rec_buff.decode())
            return 0
        else:
            print(rec_buff.decode())
            return 1
    else:
        print(cmd + ' no responce')


# Send AT command and return response information
def send_at_wait_resp(cmd, back, timeout=2000):
    rec_buff = b''
    Pico_SIM7080G.write((cmd + '\r\n').encode())
    prvmills = utime.ticks_ms()
    while (utime.ticks_ms() - prvmills) < timeout:
        if Pico_SIM7080G.any():
            rec_buff = b"".join([rec_buff, Pico_SIM7080G.read(1)])
    if rec_buff != '':
        if back not in rec_buff.decode():
            print(cmd + ' back:\t' + rec_buff.decode())
        else:
            print(rec_buff.decode())
    else:
        print(cmd + ' no responce')
    print("Response information is: ", rec_buff)
    return rec_buff


# Module startup detection
def check_start():
    # simcom module uart may be fool,so it is better to send much times when it starts.
    send_at("AT", "OK")
    utime.sleep(1)
    for i in range(1, 4):
        if send_at("AT", "OK") == 1:
            print('------SIM7080G is ready------\r\n')
            break
        else:
            module_power()
            print('------SIM7080G is starting up, please wait------\r\n')
            utime.sleep(5)


def set_network():
    print("Setting to NB-IoT mode:\n")
    send_at("AT+CFUN=0", "OK")
    send_at("AT+CNMP=38", "OK")  # Select LTE mode
    send_at("AT+CMNB=2", "OK")  # Select NB-IoT mode,if Cat-M，please set to 1
    send_at("AT+CFUN=1", "OK")
    utime.sleep(5)


# Check the network status
def check_network():
    if send_at("AT+CPIN?", "READY") != 1:
        print("------Please check whether the sim card has been inserted!------\n")
    for i in range(1, 10):
        if send_at("AT+CGATT?", "1"):
            print('------SIM7080G is online------\r\n')
            break
        else:
            print('------SIM7080G is offline, please wait...------\r\n')
            utime.sleep(5)
            continue
    send_at("AT+CSQ", "OK")
    send_at("AT+CPSI?", "OK")
    send_at("AT+COPS?", "OK")
    get_resp_info = str(send_at_wait_resp("AT+CGNAPN", "OK"))
    # getapn = get_resp_info.split('\"')
    # print(getapn[1])
    getapn1 = get_resp_info[get_resp_info.find('\"')+1:get_resp_info.rfind('\"')]
    print(getapn1)
    send_at("AT+CNCFG=0,1,\""+getapn1+"\"", "OK")
    send_at('AT+CNACT=0,1', 'OK')
    send_at('AT+CNACT?', 'OK')


def at_test():
    print("---------------------------SIM7080G AT TEST---------------------------")
    while True:
        try:
            command_input = str(input('Please input the AT command,press Ctrl+C to exit:\000'))
            send_at(command_input, 'OK', 2000)
        except KeyboardInterrupt:
            print('\n------Exit AT Command Test!------\r\n')
            module_power()
            print("------The module is power off!------\n")
            break


# SIM7080G main program
led_blink()
check_start()
set_network()
check_network()
at_test()
