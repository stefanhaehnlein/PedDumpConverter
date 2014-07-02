// stringTest.cpp : main project file.

#include "stdafx.h"
#include <math.h>


using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Threading;
using namespace System::Diagnostics;
using namespace System::ComponentModel;

public enum dumpfileTypes
{
	ascii, binary
};

public enum KeyInterpolations
{
	linear, catmull
};

class dumpFileReaderSettings
{
public:
	 int fps;
};

public ref class Vector2
{
	double x;
	double y;
};

public ref class Vector3{
public:
	double x;
	double y;
	double z;
	Vector3()	
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;
	}

	void normalize()
	{
		double l = Math::Sqrt(x*x + y*y + z*z);
		
		x = x / l;

		y = y / l;

		z = z / l;
	}
};

public ref class Key
{
public:
	double time;
	double value;

	Vector2 inVec;
	Vector2 outVec;

	Key()
	{
	}
};

ref class keyComparer: public IComparer<Key^>
{
public:

   // Calls CaseInsensitiveComparer.Compare with the parameters reversed.
	virtual int Compare( Key ^x, Key ^y) 
   {
	  /* Key^ xkey = static_cast<Key^>(x);
	   Key^ ykey = static_cast<Key^>(x);*/

	   if (x->time < y->time)
	   {
			return -1;
	   }

	   if (x->time > y->time)
	   {
			return 1;
	   }

	   return 0;
   }

};

public ref class Bezier
{
	//list<bool> ^bools;
	//vector<bool> bools;

	
public:
	KeyInterpolations keyInterpolation;

	List<Key^> ^Keys;

	IComparer<Key^> ^comp;
	
	double getValue(double inTime)
	{
		//return 0 if no keys present in controller
		if (Keys->Count < 1)
		{
			//Console::WriteLine("no keys");
			return 0.0;
		}

		//return value of first key if time is before first key
		if (inTime < Keys[0]->time)
		{
			//Console::WriteLine("before first key");
			return Keys[0]->value;
		}
		
		//return value of last key if time is after last key
		if (inTime > Keys[Keys->Count-1]->time)
		{
			//Console::WriteLine("after last key");
			return Keys[Keys->Count-1]->value;
		}
		
		int keyIndexbeforeTime = 0;

		int keyIndexAfterTime = Keys->Count-1;

		for(int i = 0; i < Keys->Count; i++)
		{
			if (Keys[i]->time == inTime)
			{
				return Keys[i]->value;
			}

			//if ((inTime > Keys[i]->time) && (Keys[i]->time > Keys[keyIndexbeforeTime]->time))
			//if (inTime > Keys[i]->time)
			{
				//keyIndexbeforeTime = i;
			}

			//if (inTime < Keys[i]->time && Keys[i]->time < Keys[keyIndexAfterTime]->time)
			if (inTime > Keys[i]->time)
			{
				keyIndexAfterTime = i+1;
				keyIndexbeforeTime = i;
			}
		}

		

		double Time1 = Keys[keyIndexbeforeTime]->time;

		double Time2 = Keys[keyIndexAfterTime]->time;

		double Time2weight = (inTime - Time1) / (Time2 - Time1);

		
		double Time1weight = 1.0 - Time2weight;
		
		//Console::WriteLine(Time1weight);
		//Console::WriteLine(Time2weight);

		if (keyInterpolation == KeyInterpolations::linear)
		{
		// linear Interpolation
			double res = Time1weight*Keys[keyIndexbeforeTime]->value + Time2weight*Keys[keyIndexAfterTime]->value;
			return res;
		}

		if (keyInterpolation == KeyInterpolations::catmull)
		{
			//catmull interpolation

			double P1 = Keys[keyIndexbeforeTime]->value;
			double P0 = Keys[keyIndexbeforeTime]->value;
			double P2 = Keys[keyIndexbeforeTime+1]->value;
			double P3 = Keys[keyIndexbeforeTime+1]->value;

			if (keyIndexbeforeTime + 2 <Keys->Count)
			{
				P3 = Keys[keyIndexbeforeTime+2]->value;
			}

			if (keyIndexbeforeTime > 0)
			{
				P0 = Keys[keyIndexbeforeTime-1]->value;
			}

			double res = 0.5 *((2 * P1) + (-P0 + P2) * Time2weight + (2*P0 - 5*P1 + 4*P2 - P3) * Time2weight*Time2weight + (-P0 + 3*P1- 3*P2 + P3) * Time2weight*Time2weight*Time2weight);

			return res;
		}

		return 0.0;
	}
	
	void addKey(double inTime, double inValue)
	{
		Key^ NewKey = gcnew Key;

		NewKey->time = inTime;

		NewKey->value = inValue;

		Keys->Add(NewKey);

		Keys->Sort(comp);
	}

    Bezier()
	{
		Keys = gcnew List<Key^>();
		comp = gcnew keyComparer();

		keyInterpolation = KeyInterpolations::linear;
	}
};

public ref class XYZBezier
{
public:
	Bezier x;
	Bezier y;
	Bezier z;

	void addKey(double inTime, double inXValue, double inYValue, double inZValue)
	{
		x.addKey(inTime, inXValue);
		y.addKey(inTime, inYValue);
		z.addKey(inTime, inZValue);
	}

	Vector3^ getValue(double inTime)
	{
		Vector3^ Val = gcnew Vector3();

		Val->x = x.getValue(inTime);
		Val->y = y.getValue(inTime);
		Val->z = z.getValue(inTime);
		
		return Val;
	}

	XYZBezier()
	{
		x.keyInterpolation = KeyInterpolations::catmull;
		y.keyInterpolation = KeyInterpolations::catmull;
		z.keyInterpolation = KeyInterpolations::catmull;
	
	}
};

public ref class PedValue
{	
public:
	
	double time;

	int id;

	double positionX;
	double positionY;
	double positionZ;

	double directionX;
	double directionY;

	int pedInfo1;
	
	int pedInfo2;

	double size;
	double loiter;
};

public ref class PedAnimationHolder
{
public:
	
	bool isLiving;
	
	int id;

	int pedInfo1;

	int pedInfo2;

	XYZBezier^ positionController;

	XYZBezier^ rotationController;

	Bezier^ walkedDistance;

	Bezier^ visibility;

	double loiter;
	
	PedAnimationHolder()
	{		
		isLiving = false;	

		id = -1;

		positionController = gcnew XYZBezier();

		rotationController = gcnew XYZBezier();

		visibility = gcnew Bezier();

		walkedDistance = gcnew Bezier();

	}

	void addKeys(PedValue^ inPedValue)
	{
		// standard param assign
		if (!isLiving)
		{
			id = inPedValue->id;
			
			pedInfo1 = inPedValue->pedInfo1;

			pedInfo2 = inPedValue->pedInfo2;	

			loiter = inPedValue->loiter;

			isLiving = true;
		}
		
		positionController->addKey(inPedValue->time,inPedValue->positionX,inPedValue->positionY,inPedValue->positionZ);

		rotationController->addKey(inPedValue->time, inPedValue->directionX, inPedValue->directionY,0.0);		
	}

	void computeVisibility()
	{
		
		if (positionController->x.Keys->Count >= 4)
		{
			visibility->addKey(positionController->x.Keys[0]->time, 0.0);
			visibility->addKey(positionController->x.Keys[1]->time, 1.0);

			visibility->addKey(positionController->x.Keys[(positionController->x.Keys->Count)-2]->time, 1.0);
			visibility->addKey(positionController->x.Keys[(positionController->x.Keys->Count)-1]->time, 0.0);
		}
	}

	void computeWalkedDistance()
	{
		if (positionController->x.Keys->Count >= 4)
		{
			double startTime = positionController->x.Keys[0]->time;

			double endTime = positionController->x.Keys[positionController->x.Keys->Count-1]->time;

			double sumDistance = 0.0;
			
			Vector3^ referencePos = positionController->getValue(startTime);			

			//for(double i = startTime; startTime <= endTime; i+=(1.0/25.0))
			for(double i = startTime; i <= endTime; i++)
			{
				Vector3^ newPos = positionController->getValue(i);

				Vector3^ distanceVector = gcnew Vector3();

				distanceVector->x = newPos->x - referencePos->x;
				distanceVector->y = newPos->y - referencePos->y;
				distanceVector->z = newPos->z - referencePos->z;				

				double distance = sqrt(distanceVector->x*distanceVector->x + distanceVector->y*distanceVector->y + distanceVector->z*distanceVector->z);

				sumDistance = sumDistance + distance;

				walkedDistance->addKey(i,sumDistance);				
			}
		}
	
	}

	void computeRotation()
	{
		Vector3^ dir1 = gcnew Vector3();

		Vector3^ dir2 = gcnew Vector3();
		dir2->x = 1.0;
		dir2->y = 0.0;
		dir2->z = 0.0;

		for (int i = 0; i < rotationController->x.Keys->Count; i++)
		{
			dir1 = rotationController->getValue(rotationController->x.Keys[i]->time);

			dir1->normalize();

			double dotProduct = dir1->x*dir2->x + dir1->y*dir2->y + dir1->z*dir2->z;

			double zRot = Math::Acos(dotProduct)*180.0 / Math::PI;

			if (dir1->y<0)
			{
				zRot = -zRot;
			}

			rotationController->z.Keys[i]->value = zRot;
		}
	}
};

public ref class DumpFileReader
{
public:
       
	   //array<PedAnimationHolder^> ^pedAnimations;
	   List<PedAnimationHolder^> ^pedAnimations;
       array<String^> ^dumpFiles;
	   dumpfileTypes dumpFileType; 
	   int biggestPedID;
	   double startTime;
	   double endTime;


	   void deleteEmptyPeds()
	   {
		   List<PedAnimationHolder^> ^noEmptyList = gcnew List<PedAnimationHolder^>();

		   for(int i = 0; i < pedAnimations->Count; i++)
		   {
			   if (pedAnimations[i]->positionController->x.Keys->Count > 0)
			   {
				   noEmptyList->Add(pedAnimations[i]);
			   }			   
		   }
	   
		   pedAnimations = noEmptyList;
	   }

       void getDumpFileList(String^ oneDumpFile)
	   {
		   dumpFiles = Directory::GetFiles(Path::GetDirectoryName(oneDumpFile) + "\\", Path::GetFileNameWithoutExtension(oneDumpFile) + ".*" ,SearchOption::TopDirectoryOnly);

		   Array::Sort(dumpFiles);
	   }

       void getDumpFileType(String^ oneDumpFile)
       {
              if ((Path::GetFileNameWithoutExtension(oneDumpFile)) == "pedvtcbdmp")
			  {
					dumpFileType = binary;

					Console::WriteLine("dumpFileType is: binary");
			  }

			  if ((Path::GetFileNameWithoutExtension(oneDumpFile)) == "pedvtcdmp")
			  {
				  dumpFileType = ascii;

				  Console::WriteLine("dumpFileType is: ascii");
			  }			  
       }

	   void getBiggestPedID()
		{

			int tempHighestID = 0;

			if (dumpFileType == ascii)
			{
				for (int i = 0; i < dumpFiles->Length; i++)
				{
					//Console::WriteLine(dumpFiles[i]);

					array<String^> ^FContent;

					FContent = File::ReadAllLines(dumpFiles[i]);
					
					//Console::WriteLine(FContent->Length);

					for (int k = 1; k < FContent->Length; k++)
					{
						array<String^> ^LineContent;
						
						array<Char> ^SplitOP = gcnew array<Char>{' '};
						
						LineContent = FContent[k]->Split(SplitOP,StringSplitOptions::RemoveEmptyEntries);

						int tempID = Convert::ToInt32(LineContent[0]);

						if (tempID > tempHighestID)
						{
							tempHighestID = tempID;
						}
					}
				}		
			}

			biggestPedID = tempHighestID;

			Console::WriteLine("highestPedID: " + biggestPedID);
		}

	   void readASCIIPedDump(String^ fName)
	   {
		   Console::WriteLine("reading: " + fName);

		   array<Char> ^SplitOP = gcnew array<Char>{' '};

		   array<String^> ^FContent;

		   FContent = File::ReadAllLines(fName);

		   array<String^> ^LineContent;
								   						
		   LineContent = FContent[0]->Split(SplitOP, StringSplitOptions::RemoveEmptyEntries);
		   
		   double Time = Convert::ToDouble(LineContent[1]->Replace('.',','));

		   if (Time< startTime)
		   {startTime = Time;}

		   if (Time > endTime)
		   {endTime = Time;}

		   Console::WriteLine("Time is: " + Time);

		   for (int i = 1; i < FContent->Length; i++ )
		   {
			   PedValue^ MyPedValue = gcnew PedValue();

			   String^ Line =  FContent[i]->Replace('.',',');

			   array<String^> ^LineContent;

			   LineContent = Line->Split(SplitOP, StringSplitOptions::RemoveEmptyEntries);

			   MyPedValue->time = Time;

			   MyPedValue->id = Convert::ToInt32(LineContent[0]);

			   MyPedValue->pedInfo1 = Convert::ToInt32(LineContent[1]);

			   MyPedValue->positionX = Convert::ToDouble(LineContent[2]);

			   MyPedValue->positionY = Convert::ToDouble(LineContent[3]);

			   MyPedValue->positionZ = Convert::ToDouble(LineContent[4]);

			   MyPedValue->directionX =  Convert::ToDouble(LineContent[5]);

			   MyPedValue->directionY = Convert::ToDouble(LineContent[6]);

			   MyPedValue->size = Convert::ToDouble(LineContent[7]);

			   MyPedValue->loiter = Convert::ToDouble(LineContent[8]);

			   MyPedValue->pedInfo2 = Convert::ToInt32(LineContent[9]);

			   pedAnimations[MyPedValue->id - 1]->addKeys(MyPedValue);
		   }
	   }

	   void readBinaryPedDump(String^ fName)
	   {

	   }

	   void readAllPedDumps()
	   {
		   pedAnimations = gcnew List<PedAnimationHolder^>();

		   for(int i = 0; i < biggestPedID; i++)
		   {
			   PedAnimationHolder^ a = gcnew PedAnimationHolder();
			   pedAnimations->Add(a);
		   }

		   for(int i = 0; i < dumpFiles->Length; i++)
		   {
			   if (dumpFileType == ascii)
			   {
				   readASCIIPedDump(dumpFiles[i]);
			   }

			   if (dumpFileType == binary)
			   {
				   readBinaryPedDump(dumpFiles[i]);
			   }
		   }
	   }

	   void computeValues()
	   {
		   for(int i = 0; i < pedAnimations->Count; i++)
		   {
			   pedAnimations[i]->computeVisibility();
			   pedAnimations[i]->computeWalkedDistance();	
			   pedAnimations[i]->computeRotation();
		   }	   
	   }

	   DumpFileReader(String^ oneDumpFile)
       {
			  startTime = 1000000.0;
			  endTime = 0.0;

			  //setting ASCII or BINARY
			  getDumpFileType(oneDumpFile);

			  //getting fileList of all relevant DumpFiles
              getDumpFileList(oneDumpFile);          

			  //searching for the highrest ID for global Initialisation
			  getBiggestPedID();

			  //reading content of all 
			  readAllPedDumps();	

			  //deleting all empty peds
			  deleteEmptyPeds();

			  //compute visibility + walked distance
			  computeValues();
       }
};


public ref class OneDumpWriter
{
	String ^ fName;
	DumpFileReader^ data;
	double time;
	
	void write(Object^ Sender, DoWorkEventArgs^ e )
	{
		Vector3^ pos;
		Vector3^ rot;
		double visibility;
		double walked;

		StreamWriter^ file =  gcnew StreamWriter(fName);

		for (int k = 0; k < data->pedAnimations->Count; k++)
		{
			pos = data->pedAnimations[k]->positionController->getValue(time);
			rot = data->pedAnimations[k]->rotationController->getValue(time);
			visibility = data->pedAnimations[k]->visibility->getValue(time);
			walked = data->pedAnimations[k]->walkedDistance->getValue(time);

			String^ line = data->pedAnimations[k]->id + "\t" + pos->x.ToString() + "\t" + pos->y.ToString() + "\t" + pos->z.ToString()  + "\t" + rot->z.ToString() + "\t" + visibility.ToString() + "\t" + walked.ToString();
			
			file->WriteLine(line);
		}
			
		file->Close();	
	}

public:
	
	BackgroundWorker^ worker;

	void writeAsync()
	{	
			worker->DoWork +=	gcnew DoWorkEventHandler(this, &OneDumpWriter::write);

			worker->RunWorkerAsync();
	}

	OneDumpWriter(String^ inFName, DumpFileReader^ inData, double inTime)
	{
		worker = gcnew BackgroundWorker();
		fName = inFName;
		data = inData;
		time = inTime;		
	}
};

public ref class DumpFileWriter
{
public:
	
	DumpFileReader^ data;

	String^ dumpDir;

	DumpFileWriter(DumpFileReader^ inData, String^ inDir)
	{
		data = inData;	
		dumpDir = inDir;
	}

	String^ intToString(int inInt, int length)
	{
		String^ word = inInt.ToString();

		int l = length - word->Length;

		for (int i = 1; i <=l; i++)
		{
			word = "0" + word;
		}
	
		return word;
	}


	void writeAllDumps()
	{
		double frameRate = 25.0;

		double frames = (data->endTime - data->startTime)* frameRate;

		

		Directory::CreateDirectory(dumpDir);

		int threadNumber = 8;

		List<OneDumpWriter ^> ^workerList= gcnew List<OneDumpWriter ^>();

		

		for (int i = 0; i < threadNumber; i++)
		{
			workerList->Add(gcnew OneDumpWriter("",data,0.0));
		}

		Console::WriteLine("initiated " + workerList->Count.ToString() + " worker(s)");

		for (int i = 0; i <= frames; i++)
		{
			double time = data->startTime + (1.0/frameRate) * i;

			String^ fName = dumpDir  + "dump" + intToString(i,4)+ ".apd";

			bool idleWorkerFound;

			//setting the slots
			do
			{
				bool continueLoop = true;

				idleWorkerFound = false;
				for (int k = 0; k < workerList->Count; k++)
				{
					if (continueLoop)
					{
						if (!workerList[k]->worker->IsBusy)
						{
							continueLoop = false;

							idleWorkerFound = true;
						
							workerList[k] = gcnew OneDumpWriter(fName, data, time);
						
							Console::WriteLine("writing dump: " + i.ToString());

							workerList[k]->writeAsync();
						}
					}
				}

			}
			while (!idleWorkerFound);					
		}
	}
};



int main(array <System::String ^> ^args)
{
    Console::WriteLine("PedDumpConverter");

		Bezier^ ab = gcnew Bezier();

		ab->addKey(1.0,1.0);
		ab->addKey(0.0,0.0);

		Console::WriteLine(ab->getValue(0.0));

		Console::WriteLine(ab->getValue(0.5));

	   String^ inputFile = "E:\\450kASCII\\pedvtcdmp.0000";
	   //String^ inputFile = "P:\\project\\0143_Pedflow\\20_working\\3d\\Pedflow\\2013-07-23_TempMataf\\pedvtcdmp.0000";

	   Stopwatch^ readStopW = gcnew Stopwatch();

	   readStopW->Start();

       DumpFileReader^ reader = gcnew  DumpFileReader(inputFile);

	   Console::WriteLine("reading took: " + readStopW->Elapsed.Hours + "h " + readStopW->Elapsed.Minutes + "m "  + readStopW->Elapsed.Seconds + "s");

	   

	   DumpFileWriter^ writer = gcnew DumpFileWriter(reader,"E:\\testDump\\");

	   Console::WriteLine("writing dumps");

	   readStopW->Reset();

	   readStopW->Start();

	   writer->writeAllDumps();
	   	  
	   Console::WriteLine("writing took: " + readStopW->Elapsed.Hours + "h " + readStopW->Elapsed.Minutes + "m "  + readStopW->Elapsed.Seconds + "s");
	   
       Console::ReadKey();

    return 0;
}
  