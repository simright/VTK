/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkInformationExecutivePortVectorKey.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkInformationExecutivePortVectorKey.h"

#include "vtkExecutive.h"
#include "vtkGarbageCollector.h"

#include <vtkstd/algorithm>
#include <vtkstd/vector>

vtkCxxRevisionMacro(vtkInformationExecutivePortVectorKey, "1.1");

//----------------------------------------------------------------------------
vtkInformationExecutivePortVectorKey::vtkInformationExecutivePortVectorKey(const char* name, const char* location):
  vtkInformationKey(name, location)
{
  vtkFilteringInformationKeyManager::Register(this);
}

//----------------------------------------------------------------------------
vtkInformationExecutivePortVectorKey::~vtkInformationExecutivePortVectorKey()
{
}

//----------------------------------------------------------------------------
void vtkInformationExecutivePortVectorKey::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
class vtkInformationExecutivePortVectorValue: public vtkObjectBase
{
public:
  vtkTypeMacro(vtkInformationExecutivePortVectorValue, vtkObjectBase);
  vtkstd::vector<vtkExecutive*> Executives;
  vtkstd::vector<int> Ports;

  virtual ~vtkInformationExecutivePortVectorValue();
  void UnRegisterAllExecutives();
};

//----------------------------------------------------------------------------
vtkInformationExecutivePortVectorValue
::~vtkInformationExecutivePortVectorValue()
{
  // Remove all our references to executives before erasing the vector.
  this->UnRegisterAllExecutives();
}

//----------------------------------------------------------------------------
void vtkInformationExecutivePortVectorValue::UnRegisterAllExecutives()
{
  for(vtkstd::vector<vtkExecutive*>::iterator i = this->Executives.begin();
      i != this->Executives.end(); ++i)
    {
    if(vtkExecutive* e = *i)
      {
      e->UnRegister(0);
      }
    }
}

//----------------------------------------------------------------------------
void vtkInformationExecutivePortVectorKey::Append(vtkInformation* info,
                                                  vtkExecutive* executive,
                                                  int port)
{
  if(vtkInformationExecutivePortVectorValue* v =
     vtkInformationExecutivePortVectorValue::SafeDownCast(
       this->GetAsObjectBase(info)))
    {
    // The entry already exists.  Append to its vector.
    executive->Register(0);
    v->Executives.push_back(executive);
    v->Ports.push_back(port);
    }
  else
    {
    // The entry does not yet exist.  Just create it.
    this->Set(info, &executive, &port, 1);
    }
}

//----------------------------------------------------------------------------
void vtkInformationExecutivePortVectorKey::Remove(vtkInformation* info,
                                                  vtkExecutive* executive,
                                                  int port)
{
  if(vtkInformationExecutivePortVectorValue* v =
     vtkInformationExecutivePortVectorValue::SafeDownCast(
       this->GetAsObjectBase(info)))
    {
    // The entry exists.  Find this executive/port pair and remove it.
    for(unsigned int i=0; i < v->Executives.size(); ++i)
      {
      if(v->Executives[i] == executive && v->Ports[i] == port)
        {
        v->Executives.erase(v->Executives.begin()+i);
        v->Ports.erase(v->Ports.begin()+i);
        executive->UnRegister(0);
        break;
        }
      }

    // If the last entry was removed, remove the entire value.
    if(v->Executives.empty())
      {
      this->SetAsObjectBase(info, 0);
      }
    }
}

//----------------------------------------------------------------------------
void vtkInformationExecutivePortVectorKey::Set(vtkInformation* info,
                                               vtkExecutive** executives,
                                               int* ports, int length)
{
  if(executives && ports && length > 0)
    {
    // Register our references to all the given executives.
    for(int i=0; i < length; ++i)
      {
      if(executives[i])
        {
        executives[i]->Register(0);
        }
      }

    // Store the vector of pointers.
    vtkInformationExecutivePortVectorValue* oldv =
      vtkInformationExecutivePortVectorValue::SafeDownCast(
        this->GetAsObjectBase(info));
    if(oldv && static_cast<int>(oldv->Executives.size()) == length)
      {
      // Replace the existing value.
      oldv->UnRegisterAllExecutives();
      vtkstd::copy(executives, executives+length, oldv->Executives.begin());
      vtkstd::copy(ports, ports+length, oldv->Ports.begin());
      }
    else
      {
      // Allocate a new value.
      vtkInformationExecutivePortVectorValue* v =
        new vtkInformationExecutivePortVectorValue;
      this->ConstructClass("vtkInformationExecutivePortVectorValue");
      v->Executives.insert(v->Executives.begin(), executives, executives+length);
      v->Ports.insert(v->Ports.begin(), ports, ports+length);
      this->SetAsObjectBase(info, v);
      v->Delete();
      }
    }
  else
    {
    this->SetAsObjectBase(info, 0);
    }
}

//----------------------------------------------------------------------------
vtkExecutive**
vtkInformationExecutivePortVectorKey::GetExecutives(vtkInformation* info)
{
  vtkInformationExecutivePortVectorValue* v =
    vtkInformationExecutivePortVectorValue::SafeDownCast(
      this->GetAsObjectBase(info));
  return v?(&v->Executives[0]):0;
}

//----------------------------------------------------------------------------
int* vtkInformationExecutivePortVectorKey::GetPorts(vtkInformation* info)
{
  vtkInformationExecutivePortVectorValue* v =
    vtkInformationExecutivePortVectorValue::SafeDownCast(
      this->GetAsObjectBase(info));
  return v?(&v->Ports[0]):0;
}

//----------------------------------------------------------------------------
void vtkInformationExecutivePortVectorKey::Get(vtkInformation* info,
                                               vtkExecutive** executives,
                                               int* ports)
{
  if(vtkInformationExecutivePortVectorValue* v =
     vtkInformationExecutivePortVectorValue::SafeDownCast(
       this->GetAsObjectBase(info)))
    {
    vtkstd::copy(v->Executives.begin(), v->Executives.end(), executives);
    vtkstd::copy(v->Ports.begin(), v->Ports.end(), ports);
    }
}

//----------------------------------------------------------------------------
int vtkInformationExecutivePortVectorKey::Length(vtkInformation* info)
{
  vtkInformationExecutivePortVectorValue* v =
    vtkInformationExecutivePortVectorValue::SafeDownCast(
      this->GetAsObjectBase(info));
  return v?static_cast<int>(v->Executives.size()):0;
}

//----------------------------------------------------------------------------
int vtkInformationExecutivePortVectorKey::Has(vtkInformation* info)
{
  vtkInformationExecutivePortVectorValue* v =
    vtkInformationExecutivePortVectorValue::SafeDownCast(
      this->GetAsObjectBase(info));
  return v?1:0;
}

//----------------------------------------------------------------------------
void vtkInformationExecutivePortVectorKey::Copy(vtkInformation* from,
                                                vtkInformation* to)
{
  this->Set(to, this->GetExecutives(from), this->GetPorts(from),
            this->Length(from));
}

//----------------------------------------------------------------------------
void vtkInformationExecutivePortVectorKey::Remove(vtkInformation* info)
{
  this->Superclass::Remove(info);
}

//----------------------------------------------------------------------------
void
vtkInformationExecutivePortVectorKey::Report(vtkInformation* info,
                                             vtkGarbageCollector* collector)
{
  if(vtkInformationExecutivePortVectorValue* v =
     vtkInformationExecutivePortVectorValue::SafeDownCast(
       this->GetAsObjectBase(info)))
    {
    for(vtkstd::vector<vtkExecutive*>::iterator i = v->Executives.begin();
        i != v->Executives.end(); ++i)
      {
      vtkGarbageCollectorReport(collector, *i, this->GetName());
      }
    }
}

//----------------------------------------------------------------------------
vtkExecutive**
vtkInformationExecutivePortVectorKey
::GetExecutivesWatchAddress(vtkInformation* info)
{
  if(vtkInformationExecutivePortVectorValue* v =
     vtkInformationExecutivePortVectorValue::SafeDownCast(
       this->GetAsObjectBase(info)))
    {
    return &v->Executives[0];
    }
  return 0;
}

//----------------------------------------------------------------------------
int*
vtkInformationExecutivePortVectorKey
::GetPortsWatchAddress(vtkInformation* info)
{
  if(vtkInformationExecutivePortVectorValue* v =
     vtkInformationExecutivePortVectorValue::SafeDownCast(
       this->GetAsObjectBase(info)))
    {
    return &v->Ports[0];
    }
  return 0;
}
