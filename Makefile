# Runs server and a client
all:
	sbt "runMain tpip.BlaaHund dbaa8026.ngrok.io"

server:
	sbt "runMain tpip.App"

client:
	sbt "runMain tpip.BlaaHundClient dbaa8026.ngrok.io"


app:
	sbt "runMain tpip.App dbaa8026.ngrok.io"
