Anmerkung zum Aufbau des DHT Rings: Gleichzeitiges Join funktioniert in dieser Version nicht immer. Falls der Ring falsch aufgebaut wurde, muss das Skript, welches den Ring automatisch aufbaut, nochmal gestartet werden.

Anmerkung zum Aufbau der Fingertables: Dieser Peer baut sich die Fingertable nicht über externe Anfragen auf. Stattdessen werden "künstliche" get Anfragen quer durch den Ring geschickt, wobei nur die Starteinträge der Fingertable als Key benutzt werden. Der entsprechende Peer antwortet dem Requester und der kann sich so seine Fingertable aufbauen.
