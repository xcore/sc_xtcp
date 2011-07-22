use File::Find;

$cmd_end=0;
$cmd_setdata=1;
$cmd_readdata=2;
$cmd_macaddr=3;
$cmd_ipaddr=4;
$cmd_timer=5;
$cmd_buttonA=6;
$cmd_buttonB=7;
$cmd_ipmethod=8;
$cmd_defaultipmethod=9;
$cmd_defaultipaddr=10;
$cmd_bigfile=11;
$cmd_defaultnetmask=12;
$cmd_serialnum=13;
$cmd_firmware=14;

%files = ();

sub process {
    $binary = -1;
    if (/.*\.(html|css)$/) {
        $binary = 0;
        $content = "text/$1";
#        print "text: $File::Find::dir/$_\n";
    }
    if (/.*\.(jpg|gif|png)$/) {
        $content = "image/$1";
        $binary = 1;
        
    }

    if ($binary != -1) {    
        @cmds = ();
        $name = "$File::Find::dir/$_";
        open(FILE, "<$_");
        $name  =~ s/\.\//\//g;
        $id = $name;
        $id =~ s/\.|\//_/g;
        print "static unsigned char data_${id}[] = {\n";
        print "    /* $name */\n";
        $namelen = length($name)+1;
        print "    /* $namelen */\n";
        $colcount = 0;
        $prev = "";
        $bytecount = -$namelen;
        

        $files{$id} = $namelen;
               
        print "    ";
        @lines = <FILE>;
unshift(@lines,"HTTP/1.0 200 OK
Server: xc2/pre-1.0 (http://xmos.com)
Content-type: $content

");
        push(@cmds,$cmd_setdata);
        push(@cmds,"(unsigned int) &data_${id}[$namelen]");
        unshift(@lines,"$name\0");
        foreach $line (@lines) {    
            if ($binary==0) {
                $val = chr($cmd_macaddr+200);
                $line =~ s/%%MACADDRESS%%/$val/g;               
                $val = chr($cmd_timer+200);
                $line =~ s/%%TIMER%%/$val/g;
                $val = chr($cmd_buttonA+200);
                $line =~ s/%%BUTTONA%%/$val/g;
                $val = chr($cmd_buttonB+200);
                $line =~ s/%%BUTTONB%%/$val/g;
                $val = chr($cmd_ipaddr+200);
                $line =~ s/%%IPADDR%%/$val/g;
                $val = chr($cmd_ipmethod+200);
                $line =~ s/%%IPMETHOD%%/$val/g;
                $val = chr($cmd_defaultipmethod+200);
                $line =~ s/%%DEFAULTIPMETHOD%%/$val/g;
                $val = chr($cmd_defaultipaddr+200);
                $line =~ s/%%DEFAULTIPADDR%%/$val/g;
                $val = chr($cmd_bigfile+200);
                $line =~ s/%%BIGFILE%%/$val/g;
                $val = chr($cmd_defaultnetmask+200);
                $line =~ s/%%DEFAULTNETMASK%%/$val/g;
                $val = chr($cmd_serialnum+200);
                $line =~ s/%%SERIALNUM%%/$val/g;
                $val = chr($cmd_firmware+200);
                $line =~ s/%%FIRMWAREVERSION%%/$val/g;
            }

            @ss = split(//,$line);
            foreach $s (@ss) {
                $skip = 0;
                if ($binary==0) {
                    if ($s =~ /\n/) {
                        if ($prev !~ /\r/) {
                            print ord("\r");
                            print ",";
                            $bytecount++;
                            $colcount++;
                        }
                    }
                    if (ord($s) > 200) {
                        push(@cmds, $cmd_readdata);
                        push(@cmds, $bytecount);
                        push(@cmds, ord($s)-200);
                        $bytecount=0;
                        $skip = 1;
                    }

                }                 
                if (!$skip) {
                    print ord($s);
                    print ",";
                    $prev = $s;                
                    $bytecount++;
                    $colcount++;
                    if ($colcount > 10) {
                        $colcount = 0;
                        print "\n    ";
                    }
                }
            }
        }
        print "\n};\n\n";
        close(FILE);

        push(@cmds,$cmd_readdata);
        push(@cmds,"$bytecount");
        push(@cmds,$cmd_end);

        print "static unsigned int cmd_${id}[] = {\n";
        foreach $cmd (@cmds) {
            print "$cmd,";
        }
        print "\n};\n\n"

    }
}


find(\&process, ("."));

$prev = "NULL";
$count = 0;
while (($file,$len) = each(%files)) {
    print "struct fsdata_file file_${file}[] = {{$prev, data_$file, cmd_$file}};\n\n";
    $prev = "file_$file";
    $count++;
}

print "#define FS_ROOT $prev\n\n";
print "#define FS_NUMFILES $count\n\n";


