# Runs server and a client
all:
	sbt "runMain tpip.BlaaHund d51eb215.ngrok.io"

server:
	sbt "runMain tpip.App"

client:
	sbt "runMain tpip.BlaaHundClient d51eb215.ngrok.io"


app:
	sbt "runMain tpip.App d51eb215.ngrok.io"

# pingrt.ngrok.io
