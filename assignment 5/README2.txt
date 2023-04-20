Q2.
Case 1:
	Two semaphore are taken, bridge and dir and initialize bridge with 1 and dir with number of threads.
	Then threads were created and assigned to northBoundPeople or soundBoundPeople function accordingly.
	In the function northBoundPeople, people first try to aquire bridge lock and then tries to aquire dir lock.
	In the function southBoundPeople, people first try to aquire dir lock and then tries to aquire bridge lock.
	
	If the bridge lock is aquired by any northbound people then it prevents southbound people from crossing the
	bridge as southbound can not aquire the bridge lock, although they aquire dir lock, and vice vresa.
	So northbound people prevents southbounds from crossing the bridge and southbound prevents northbound.
	
Case 2:
	Semaphore bridge and dir both initialized with 1. So when north bound people try to cross the bridge it needs to aquire both locks.
	Same for the southbound people. Now, if northbound aquire bridge lock and southbound aquire dir lock then the bridge is in a deadlock
	as northbound cannot move further as it require dir lock and southbound can not move further as it require bridge lock and both 
	northbound people and southbound people cannot back up as mentioned in the question.
	
Compilation:
	g++ Q2.cpp -o Q2
Run:
	./Q2 1  <for case 1>
	./Q2 2  <for case 2>

