# s-talk
Multi-threaded console chat application 

Run 'make' then 

    ./s-talk [my port number] [remote machine name] [remote port number]
    
e.g: Running 

    ./s-talk 6001 machine-name 6060
in one terminal, and 

    ./s-talk 6060 machine-name 6001
    
in another, allows one to send messages between consoles on the same machine - provided the ports are free.

Likely will not work across devices do to firewalls and other fancy security systems. 