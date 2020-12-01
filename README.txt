/**
* Natthaphong Kongkaew
* kongkaen@oregonstate.edu
* Assignment 5: One Time Pad
* CS 344 Fall_2020 Oregonstate University
*/

-------------------------------------------------------------------
Files to compile:
  1.enc_client.c
  2.enc_server.c
  3.dec_client.c
  4.dec_server.c
  5.keygen.c
  6.makeall

Compile: ./makeall

NOTE: If ./makeall shows permission denied error,
changing permission by running command: chmod 777 compileall

-------------------------------------------------------------------
Files to test:
  1.plaintext1
  2.plaintext2
  3.plaintext3
  4.plaintext4
  5.plaintext5
  6.p5testscript

Testing using test script: ./p5testscript RANDOM_PORT1 RANDOM_PORT2 > mytestresults 2>&1

Testing by using command line: example usage;
cat plaintext1
THE RED GOOSE FLIES AT MIDNIGHT STOP
./enc_server 57171 &
./dec_server 57172 &
./keygen 10
EONHQCKQ I
./keygen 10 > mykey
cat mykey
VAONWOYVXP
./keygen 10 > myshortkey
./enc_client plaintext1 myshortkey 57171 > ciphertext1
Error: key ‘myshortkey’ is too short
echo $?
1
./keygen 1024 > mykey
./enc_client plaintext1 mykey 57171 > ciphertext1
cat ciphertext1
WANAWTRLFTH RAAQGZSOHCTYS JDBEGYZQDQ
./keygen 1024 > mykey2
./dec_client ciphertext1 mykey 57172 > plaintext1_a
./dec_client ciphertext1 mykey2 57172 > plaintext1_b
cat plaintext1_a
THE RED GOOSE FLIES AT MIDNIGHT STOP
cat plaintext1_b
WSXFHCJAEISWQRNO L ZAGDIAUAL IGGTKBW
cmp plaintext1 plaintext1_a
echo $?
0
cmp plaintext1 plaintext1_b
plaintext1 plaintext1_b differ: byte 1, line 1
echo $?
1
./enc_client plaintext5 mykey 57171
./enc_client error: input contains bad characters
echo $?
1
./enc_client plaintext3 mykey 57172
Error: could not contact enc_server on port 57172
echo $?
2
