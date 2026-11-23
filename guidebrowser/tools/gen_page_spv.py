# Compiles src/docpage.slang with slangc and writes src/docpage_spv.cone (the SPIR-V as a Cone array).
import subprocess, struct, sys, os
here = os.path.dirname(os.path.abspath(__file__))
src = os.path.join(here, '..', 'src')
slangc = r'C:\VulkanSDK\1.4.357.0\Bin\slangc.exe'
spv = os.path.join(os.environ.get('TEMP', '.'), 'docpage.spv')
subprocess.check_call([slangc, os.path.join(src, 'docpage.slang'), '-target', 'spirv', '-profile', 'spirv_1_5',
                       '-matrix-layout-column-major', '-fvk-use-entrypoint-name', '-o', spv])
d = open(spv, 'rb').read()
words = struct.unpack('<%dI' % (len(d) // 4), d)
rows = []
for i in range(0, len(words), 6):
    rows.append('  ' + ', '.join('0x%08xu32' % w for w in words[i:i + 6]))
out = ['// Written by tools/gen_page_spv.py from docpage.slang: do not edit.',
       'let DOCPAGE_SPIRV: Array[u32, %d] = [' % len(words), ',\n'.join(rows), '];']
open(os.path.join(src, 'docpage_spv.cone'), 'w', newline='\n').write('\n'.join(out) + '\n')
print(len(words), 'words')
