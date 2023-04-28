import io
import sys

def genHvDBSubstitutions(fileName, idAndIPList):
    with open(fileName, "w") as dst:
        dst.write('file "db/pmt-hv.db" {\n')
        dst.write('pattern {prefix, configFilePath, devName, uartNode}\n')
        for gcu in idAndIPList:
            for i in range(3):
                dst.write('    {{JUNO:KS:GU{0:04d}:u{1}, file://./gcu1f3/xml_files/connections.xml, GCU1F3_{0:04d}, uart_{1}}}\n'.format(gcu[0], i))
        dst.write('}')

def genGcuDBSubstitutions(fileName, idAndIPList):
    with open(fileName, "w") as dst:
        dst.write('file "db/pmt-gcu.db" {\n')
        dst.write('pattern {prefix, configFilePath, devName}\n')
        for gcu in idAndIPList:
            dst.write('    {{JUNO:KS:GU{0:04d}, file://./gcu1f3/xml_files/connections.xml, GCU1F3_{0:04d}}}\n'.format(gcu[0]))
        dst.write('}')

def genGcuConnectionFile(fileName, idAndIPList, useControlhub):
    with open(fileName, "w") as dst:
        dst.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        dst.write('<connections>\n')
        for gcu in idAndIPList:
            if useControlhub:
                dst.write('<connection id="GCU1F3_{0:04d}" uri="chtcp-2.0://localhost:10203?target={1}:50001" address_table="file://sc.xml" />\n'.format(gcu[0], gcu[1]))
            else:
                dst.write('<connection id="GCU1F3_{0:04d}" uri="ipbusudp-2.0://{1}:50001" address_table="file://sc.xml" />\n'.format(gcu[0], gcu[1]))
        dst.write('</connections>')

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

def convertToIP(gcuId):
    part3 = gcuId / 256
    part4 = gcuId % 256
    IP = '10.7.{0}.{1}'.format(part3, part4)
    return IP

def loadGcuIdAndIPList(fileName):
    idAndIPList = []
    with open(fileName, "r") as src:
        for id in src:
            gcuId = int(id)
            gcu = []
            gcu.append(gcuId)
            IP = convertToIP(gcuId)
            gcu.append(IP)
            idAndIPList.append(gcu)
    return idAndIPList


if len(sys.argv) == 2:
    gcuIdFile = sys.argv[1]
    idAndIPList = loadGcuIdAndIPList(gcuIdFile)
    genGcuConnectionFile("connections-useControlHub.xml", idAndIPList, True)
    genGcuConnectionFile("connections-notUseControlHub.xml", idAndIPList, False)
    fn = 'pmt-gcu-kunshan.substitutions'
    genGcuDBSubstitutions(fn, idAndIPList)
    fn = 'pmt-hv-kunshan.substitutions'
    genHvDBSubstitutions(fn, idAndIPList)
else:
    print("Usage: python genTestDb-kunshan.py gcuIdListFile.txt")