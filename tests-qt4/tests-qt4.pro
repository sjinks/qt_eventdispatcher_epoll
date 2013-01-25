TEMPLATE = subdirs

SUBDIRS = \
	qeventdispatcher \
	qeventloop \
	qtimer \
	qsocketnotifier \
	events

unix: SUBDIRS += issues
