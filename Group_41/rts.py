import sys
import dpkt
import struct
import socket
import itertools

l = ['0','256','512','1024']
i = 0
out = open('ap_rts.txt','w')

while i < len(l):
	filename = 'AP'+l[i]+'-0-0.pcap'
	f = open(filename)
	print filename
	pcap = dpkt.pcap.Reader(f)

	frame_rts = 0
	frame_rts_total = 0
	for ts,buf in pcap:
		buf_radiotap = dpkt.radiotap.Radiotap(buf)
		buf_radiotap_len = socket.ntohs(buf_radiotap.length)
		wlan = dpkt.ieee80211.IEEE80211(buf[buf_radiotap_len:])

		if wlan.subtype == dpkt.ieee80211.C_RTS:
			frame_rts_total  = frame_rts_total + len(buf)
			frame_rts = frame_rts + 1
	x = (frame_rts_total*8.00)/(1024*1024*50)
	ele2 = l[i]
	out.write(str(x)+' '+ele2+'\n')
	i += 1

out.close()

out = open('sta1_rts.txt','w')
i = 0

while i < len(l):
	filename = 'STA'+l[i]+'-1-0.pcap'
	f = open(filename)
	print filename
	pcap = dpkt.pcap.Reader(f)

	frame_rts = 0
	frame_rts_total = 0
	for ts,buf in pcap:
		buf_radiotap = dpkt.radiotap.Radiotap(buf)
		buf_radiotap_len = socket.ntohs(buf_radiotap.length)
		wlan = dpkt.ieee80211.IEEE80211(buf[buf_radiotap_len:])

		if wlan.subtype == dpkt.ieee80211.C_RTS:
			frame_rts_total  = frame_rts_total + len(buf)
			frame_rts = frame_rts + 1
	x = (frame_rts_total*8.00)/(1024*1024*50)
	ele2=l[i]
	out.write(str(x)+' '+ele2+'\n')
	i += 1
out.close()


out = open('sta2_rts.txt','w')
i = 0

while i < len(l):
	filename = 'STA'+l[i]+'-2-0.pcap'
	f = open(filename)
	print filename
	pcap = dpkt.pcap.Reader(f)

	frame_rts = 0
	frame_rts_total = 0
	for ts,buf in pcap:
		buf_radiotap = dpkt.radiotap.Radiotap(buf)
		buf_radiotap_len = socket.ntohs(buf_radiotap.length)
		wlan = dpkt.ieee80211.IEEE80211(buf[buf_radiotap_len:])

		if wlan.subtype == dpkt.ieee80211.C_RTS:
			frame_rts_total  = frame_rts_total + len(buf)
			frame_rts = frame_rts + 1
	x = (frame_rts_total*8.00)/(1024*1024*50)
	ele2=l[i]
	out.write(str(x)+' '+ele2+'\n')
	i += 1

out.close()
