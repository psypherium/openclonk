/*--
	Pump
	Author: Maikel, ST-DDT
	
	Pumps liquids using drain and source pipes.
--*/

#include Library_PowerConsumer


// This object is a liquid pump, thus pipes can be connected.
public func IsLiquidPump() { return true; }

protected func Initialize()
{
	SetAction("Wait");
	return;
}

/*-- Pipe connection --*/

local source_pipe;
local drain_pipe;

// Set-Getters for source and drain pipe.
public func SetSource(object pipe) { source_pipe = pipe; }
public func GetSource() { return source_pipe; }
public func SetDrain(object pipe) { drain_pipe = pipe; }
public func GetDrain() { return drain_pipe; }

protected func OnPumpStart()
{
	if (!ReadyToPump())
		SetAction("Wait");
	return;
}

protected func OnWaitStart()
{
	if (ReadyToPump())
		SetAction("Pump");
	return;
}

local aMaterials=["", 0]; //contained liquids

protected func Pumping()
{
	// Only proceed if there is enough power available.
	if (!CheckPower(2))
		return SetAction("Wait");
	// Pump liquids.
	if (!source_pipe)
		return SetAction("Wait");
	//IsEmpty?
	if ((aMaterials[1] == 0) || (aMaterials[0] == ""))
	{
		//Get new Materials
		aMaterials = source_pipe->GetLiquid("*", 5, this, true);
		//No Material to pump?
		if ((aMaterials[0] == "") || (aMaterials[1] == 0))
			return;
	}
	if (drain_pipe)
		aMaterials[1] -= BoundBy(drain_pipe->PutLiquid(aMaterials[0], aMaterials[1], this), 0, aMaterials[1]);
	else
	{
		var i = Max(0, aMaterials[1]), itMaterial = Material(aMaterials[0]);
		while (i--)
			InsertMaterial(itMaterial);
		aMaterials = ["", 0];
	}
	//maybe add the possebility to empty pump (invaild mats?)
	return;	
}

// Returns whether the pump can pump some liquid.
private func ReadyToPump()
{
	// If there is no source pipe, return false.
	if (!source_pipe)
		return false;
	// If there is nothing to pump at the source return false.
	var source = source_pipe->GetConnectedObject(this);
	if (!source)
		return false;
	if (!source->GBackLiquid())
		return false;
	// TODO: Account for pumping into buildings.
	// Pumping is okay.
	return true;
}

local Name = "$Name$";
local ActMap = {
	Pump = {
		Prototype = Action,
		Name = "Pump",
		Procedure = DFA_NONE,
		Length = 20,
		Delay = 3,
		X = 0,
		Y = 0,
		Wdt = 28,
		Hgt = 32,
		NextAction = "Pump",
		StartCall = "OnPumpStart",
		PhaseCall = "Pumping"
	},
	Wait = {
		Prototype = Action,
		Name = "Wait",
		Procedure = DFA_NONE,
		Length = 1,
		Delay = 50,
		X = 0,
		Y = 0,
		Wdt = 28,
		Hgt = 32,
		NextAction = "Wait",
		StartCall = "OnWaitStart"
	}
};
