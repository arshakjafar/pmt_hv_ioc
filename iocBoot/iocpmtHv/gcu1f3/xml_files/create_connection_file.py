import argparse

parser = argparse.ArgumentParser()

parser.add_argument('-s', action='store', dest='gcu_num',
                    help='number of gcus connected')

results = parser.parse_args()

gcu_num = results.gcu_num

with open("connections.xml", "w+") as c:
    c.write('<?xml version="1.0" encoding="UTF-8"?>')
    c.write('\n')
    c.write('\n')
    c.write('<connections>')
    c.write('\n')
    c.write('\t<connection id="BEC" uri="ipbusudp-2.0://10.10.10.1:50001" address_table="file://BEC_example_design.xml"/>')
    c.write('\n')
    c.write('\t<connection id="GCU_default" uri="ipbusudp-2.0://10.10.10.19:50001" address_table="file://gcu1f3.xml"/>')
    c.write('\n')
    c.write('\t<connection id="sp6_default" uri="ipbusudp-2.0://10.10.10.18:50001" address_table="file://sp6_addr_table.xml"/>')
    for i in range(1, int(gcu_num) + 1):
        c.write('\n')
        id = '"GCU1F3_' + str(i) + '"'
        uri = '"chtcp-2.0://localhost:10203?target=10.10.10.' + str(18 + (i*2)) + ':50001\"'
        address= '"file://gcu1f3.xml"'
        c.write('\t<connection id=' + id + ' uri=' + uri +  ' address_table=' + address +' />')
    c.write('\n')
    c.write('</connections>')
