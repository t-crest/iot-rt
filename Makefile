# Runs server and a client

app:
	sbt "runMain tpip.App"

eclipse:
	sbt eclipse

ngrok:
	ngrok http -subdomain=iprt 8080
# pingrt.ngrok.io


serip:
	sudo ./slip
	ping 192.168.1.2
