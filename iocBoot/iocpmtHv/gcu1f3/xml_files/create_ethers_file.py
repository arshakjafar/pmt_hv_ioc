import argparse

parser = argparse.ArgumentParser()

parser.add_argument('-s', action='store', dest='gcu_num',
                    help='number of gcus connected')

results = parser.parse_args()

gcu_num = results.gcu_num

#MAC
fixed_mac_s6 = "02:0d:db:a1:aa:bb"
fixed_mac_k7 = "02:0a:db:a1:aa:bb"
var_fixed_mac_s6 = "02:0d:db:a1:00:"
var_fixed_mac_k7 = "02:0a:db:a1:00:"

#IP
fixed_ip_s6 = "10.10.10.18"
fixed_ip_k7 = "10.10.10.19"
var_fixed_ip = "10.10.10."


with open("ethers", "w+") as e:
    e.write(fixed_mac_s6 + "\t" + fixed_ip_s6 + "\t#Spartan 6 Fixed MAC")
    e.write("\n")
    e.write(fixed_mac_k7 + "\t" + fixed_ip_k7 + "\t#Kintex 7 Fixed MAC")
    e.write("\n")
    for i in range(0, int(gcu_num)):
        e.write("\n")
        e.write(var_fixed_mac_s6 + '{0:02x}'.format(1+(i)) + "\t" + var_fixed_ip + str(21 + (2*i)) + "\t# Spartan6 GCU " + str(i+1) + " MAC")
        e.write("\n")
        e.write(var_fixed_mac_k7 + '{0:02x}'.format(1+(i)) + "\t" + var_fixed_ip + str(20 + (2*i)) + "\t# Kintex7 GCU " + str(i+1) + " MAC")
        e.write("\n")

