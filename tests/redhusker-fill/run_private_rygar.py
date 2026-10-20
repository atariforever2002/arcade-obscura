from pathlib import Path
import subprocess,os,socket,json,time,shutil
from PIL import Image
root=Path(__file__).parent
emu=root/'local-emulator'
env=os.environ.copy();env['LD_LIBRARY_PATH']=str(emu/'lib')
env['QEMU_MODULE_DIR']=str(emu/'modules')
def mcopy(*args):
    subprocess.run([str(emu/'bin/mcopy'),*map(str,args)],env=env,check=True)
out=root/'rygar-results';out.mkdir(exist_ok=True)
for mode in ['off','on']:
    disk=out/(mode+'.img');shutil.copyfile(emu/'hdd.img',disk)
    boot=out/(mode+'-boot.img');shutil.copyfile(emu/'boot.img',boot)
    bat=out/(mode+'.bat')
    bat.write_bytes(('@echo off\r\nC:\r\nCD \\\r\nSET PATH=C:\\DJGPP\\BIN\r\nMAMEFILL rygar -soundcard 0 -resolution 640x480 -noscanlines '+('-filltest' if mode=='on' else '-nofilltest')+' > GAMELOG.TXT\r\necho RETURNED > DONE.TXT\r\nEXITVM\r\n').encode())
    mcopy('-o','-i',boot,bat,'::FDAUTO.BAT')
    mcopy('-o','-i',str(disk)+'@@1048576',root/'upload/RYGAR.ZIP','::RYGAR.ZIP')
    sockpath='/tmp/rygar-'+mode+'.sock'
    Path(sockpath).unlink(missing_ok=True)
    command=[str(emu/'bin/qemu-system-i386'),'-L',str(emu/'share/qemu'),'-bios',str(emu/'share/seabios/bios-256k.bin'),'-machine','pc','-cpu','pentium2','-m','64','-vga','std','-display','none','-qmp','stdio','-serial','none','-no-reboot','-device','isa-debug-exit,iobase=0xf4,iosize=0x04','-drive','file='+str(boot)+',format=raw,if=floppy','-drive','file='+str(disk)+',format=raw,if=ide','-boot','a']
    with open(out/(mode+'-qemu.log'),'w') as log:
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
        time.sleep(15)
        qmp('screendump',{'filename':str(out/(mode+'.ppm'))})
        Image.open(out/(mode+'.ppm')).save(out/(mode+'.png'))
        qmp('human-monitor-command',{'command-line':'sendkey esc'})
        try:p.wait(timeout=10)
        except subprocess.TimeoutExpired:qmp('quit');p.wait()
        print(mode,'exit',p.returncode,flush=True)
    mcopy('-o','-i',str(disk)+'@@1048576','::GAMELOG.TXT',out/(mode+'-game.txt'))
    print((out/(mode+'-game.txt')).read_text(errors='replace'),flush=True)
