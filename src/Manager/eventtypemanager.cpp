#include "eventtypemanager.h"

#include "../DataModel/opendatafile.h"
#include "../DataModel/undocommandfactory.h"

void EventTypeManager::insertRowBack()
{
	int rc = file->dataModel->eventTypeTable()->rowCount();
	file->undoFactory->insertEventType(rc, 1, "add EventType row");
}
