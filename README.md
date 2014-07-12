glouglou (v3)
========

network traffic visualisation in real time

WARNING
WARNING Development in progress, non functionnal
WARNING

Glouglou provides visualisation of network and process activity on local or
remote machines in real time.

The solution is constitued of
* a Central Server
* multiple Probes
* multiple visualisation clients

Programs
========

* The libraries:
libglouglou - undelaying library for glouglou Central Server and glouglou Probes
libgsnetstream - graphstream netstream implementation in C with extensions
libsendbuf - buffer helper for sending fixed size data

* The Central Server
glougloud

* The Probes

* The Visualisation clients

Installation - Central Server
=============================

1. libglouglou
2. libgsnetstream
3. libsendbuf
3. contrib/libwebsock
4. glougloud daemon

Installation - Probes
=====================

1. libsendbuf
2. libglouglou
3. XXX probe program

Installation - Visualisation clients
====================================

1. libgsnetstream
2. XXX visualisation clients program

