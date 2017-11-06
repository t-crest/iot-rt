# Runs server and a client

app:
	sbt "runMain tpip.App"

eclipse:
	sbt eclipse

ngrok:
	ngrok http -subdomain=iprt 8080
# pingrt.ngrok.io
