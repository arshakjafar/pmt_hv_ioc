import io
import sys

def genDBSubstitutions(fileName, count):
    with open(fileName, "w") as dst:
        dst.write('file "db/pmt-hv.db" {\n')
        dst.write('pattern {prefix, configFilePath, devName, uartNode}\n')
        for i in range(count):
            dst.write('    {ch0u' + '{0:05d}'.format(i * 3 + 1) + ', file://./gcu1f3/xml_files/connections.xml, GCU1F3_CH_0, uart_0}\n')
            dst.write('    {ch0u' + '{0:05d}'.format(i * 3 + 2) + ', file://./gcu1f3/xml_files/connections.xml, GCU1F3_CH_0, uart_1}\n')
            dst.write('    {ch0u' + '{0:05d}'.format(i * 3 + 3) + ', file://./gcu1f3/xml_files/connections.xml, GCU1F3_CH_0, uart_2}\n')
        dst.write('}')

def genPVTableForTest(fileName, count):
    with open(fileName, "w") as dst:
        dst.write('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n')
        dst.write('<pvtable enable_save_restore="true" version="3.0">\n')
        dst.write('    <timeout>60.0</timeout>\n')
        dst.write('    <pvlist>\n')
        for i in range(count):
            dst.write('        <pv>\n')
            dst.write('            <selected>true</selected>\n')
            dst.write('            <name>ch0u' + '{0:05d}'.format(i + 1) + ':hvMesValue</name>\n')
            dst.write('            <tolerance>0.1</tolerance>\n')
            dst.write('            <saved_value_timestamp/>\n')
            dst.write('            <completion>true</completion>\n')
            dst.write('        </pv>\n')
        dst.write('    </pvlist>\n')
        dst.write('</pvtable>\n')


if len(sys.argv) == 2:
    count = int(sys.argv[1])
    fn = 'testDB-{0}.substitutions'.format(count)
    genDBSubstitutions(fn, count)
    fn = 'pvTableOfHVUTest-{0}x3.pvs'.format(count)
    genPVTableForTest(fn, count * 3)
