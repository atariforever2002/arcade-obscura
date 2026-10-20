from pathlib import Path
import subprocess,os,json,time,shutil,sys
from PIL import Image
root=Path(__file__).parent
emu=root/'local-emulator'
env=os.environ.copy();env['LD_LIBRARY_PATH']=str(emu/'lib')
env['QEMU_MODULE_DIR']=str(emu/'modules')
def mcopy(*args):
    subprocess.run([str(emu/'bin/mcopy'),*map(str,args)],env=env,check=True)
out=root/os.environ.get('TEST_DIR','rygar-results');out.mkdir(exist_ok=True)
for mode in (sys.argv[1:] or ['off','on']):
    disk=out/(mode+'.img');shutil.copyfile(emu/'hdd.img',disk)
    boot=out/(mode+'-boot.img');shutil.copyfile(emu/'boot.img',boot)
    bat=out/(mode+'.bat')
    bat.write_bytes(('@echo off\r\nC:\r\nCD \\\r\nSET PATH=C:\\DJGPP\\BIN\r\nCWSDPMI -p\r\nTESTFILL > PRECHECK.TXT\r\nMAMEFILL rygar -log -soundcard 0 -resolution 640x480 -noscanlines '+('-filltest' if mode=='on' else '-nofilltest')+' > GAMELOG.TXT\r\necho RETURNED > DONE.TXT\r\nEXITVM\r\n').encode())
    if os.environ.get('VESA_MODE'):
        bat.write_bytes(bat.read_bytes().replace(b'-log ',('-log -vesamode '+os.environ['VESA_MODE']+' ').encode()))
    mcopy('-o','-i',boot,bat,'::FDAUTO.BAT')
    mcopy('-o','-i',str(disk)+'@@1048576',root/'upload/RYGAR.ZIP','::RYGAR.ZIP')
    command=[str(emu/'bin/qemu-system-i386'),'-L',str(emu/'share/qemu'),'-bios',str(emu/'share/seabios/bios-256k.bin'),'-machine','pc','-cpu','pentium2','-m','64','-vga','std','-display','none','-qmp','stdio','-serial','none','-no-reboot','-device','isa-debug-exit,iobase=0xf4,iosize=0x04','-drive','file='+str(boot)+',format=raw,if=floppy','-drive','file='+str(disk)+',format=raw,if=ide','-boot','a']
    with open(out/(mode+'-qemu.log'),'w') as log:
        command += ['-nic','none']
        p=subprocess.Popen(command,env=env,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=log)
        greeting=p.stdout.readline()
        if not greeting:raise RuntimeError((out/(mode+'-qemu.log')).read_text())
        def qmp(cmd,args=None):
            p.stdin.write((json.dumps({'execute':cmd,'arguments':args or {}})+'\n').encode());p.stdin.flush()
            while True:
                r=json.loads(p.stdout.readline())
                if 'return' in r:return r
                if 'error' in r:raise RuntimeError(r)
        qmp('qmp_capabilities')
        time.sleep(float(os.environ.get('START_DELAY','15')))
        qmp('human-monitor-command',{'command-line':'sendkey 5 1000'})
        time.sleep(1)
        qmp('human-monitor-command',{'command-line':'sendkey 1 1000'})
        time.sleep(10)
        qmp('screendump',{'filename':str(out/(mode+'.ppm'))})
        Image.open(out/(mode+'.ppm')).save(out/(mode+'.png'))
        qmp('human-monitor-command',{'command-line':'sendkey esc 1000'})
        time.sleep(2)
        if p.poll() is None:qmp('human-monitor-command',{'command-line':'sendkey ret 1000'})
        try:p.wait(timeout=10)
        except subprocess.TimeoutExpired:qmp('quit');p.wait()
        print(mode,'exit',p.returncode,flush=True)
        if p.returncode != 33:raise RuntimeError('Game did not complete its DOS batch return')
    mcopy('-o','-i',str(disk)+'@@1048576','::GAMELOG.TXT',out/(mode+'-game.txt'))
    mcopy('-o','-i',str(disk)+'@@1048576','::ERROR.LOG',out/(mode+'-error.log'))
    mcopy('-o','-i',str(disk)+'@@1048576','::DONE.TXT',out/(mode+'-done.txt'))
    assert 'RETURNED' in (out/(mode+'-done.txt')).read_text()
    print((out/(mode+'-game.txt')).read_text(errors='replace'),flush=True)
