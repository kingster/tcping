# Installation:
```bash
make install
```

# Usage:
tcping hostname
(uses port 80) or
tcping -p port hostname

ping once:
tcping -p port -c 1 hostname

tcping returns:  
0 on success  
2 if the host or service could not be resolved  
127 on other errors  

### Examples:
```bash
tcping -p 8080 127.0.0.1
tcping -p 80 -t 5 -I eth0 www.google.com
```
