// MIT License
// 
// Copyright (c) 2017 tbenzi Tullio Benzi
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ******************************
// CODE EMBRYO UNDER DEVELOPMENT
// ******************************
#ifndef NUM_STATES
ERROR_DEFINE_NUM_STATES_VALUE
#endif

typedef void    (*myStatusFunc)(void* pStructData);
typedef void    (*myDropOutFunc)(void* pStructData);
typedef void    (*myTransitionFunc)(void* pStructData);
typedef void    (*myPickUpFunc)(void* pStructData);
typedef int     (*myChangeStatusFunc)(void* pStructData);

typedef enum {  NO_ERROR                    = 0,
				ALREADY_DEFINED_STATE       = 1,
				STATE_WITHOUT_CHANGE        = 2,
				UNDFINED_CYCLE_TIME         = 3,
				IND_STATE_OVER_MAX          = 4,
				IND_STATE_OVER_MAX_IN_TRANS = 5,
				NO_CHANGE_STATE_DEFINED     = 6,
				WRONG_IND_STATE_MAX_IN_TRANS = 7,
                NEGATIVE_MAX_MS_IN_STATUS    = 8,
				MAX_NUM_ERROR 
} E_STATE_MACHINE_ERROR;

struct STR_ERR {
	const char* s;
};
STR_ERR stateMachineErrorString[MAX_NUM_ERROR] =
			{{"NO_ERROR"},
			 {"ALREADY_DEFINED_STATE"},
			 {"STATE_WITHOUT_CHANGE"},
			 {"UNDFINED_CYCLE_TIME"},
			 {"IND_STATE_OVER_MAX"},
			 {"IND_STATE_OVER_MAX_IN_TRANS"},
			 {"NO_CHANGE_STATE_DEFINED"},
			 {"WRONG_IND_STATE_MAX_IN_TRANS"},
			 {"NEGATIVE_MAX_MS_IN_STATUS"},
};

class CStateMachine
{
		myStatusFunc m_fStatus[NUM_STATES];                     // State functions vector
		myDropOutFunc m_fDropOut[NUM_STATES];                   // DropOut (from a state) functions vector
		myTransitionFunc m_fTransition[NUM_STATES][NUM_STATES]; // Transition (from a specify state to another) functions vector
		myPickUpFunc m_fPickUp[NUM_STATES];                     // PickUp (into a state) functions vector
		myChangeStatusFunc m_fChangeStatus[NUM_STATES];         // ChangeStatus functions vector
		const char* m_StatusName[NUM_STATES];                   // State names vector
		long int m_MaxMsInStatus[NUM_STATES];                        // Maximum allowed time to stack in any states vector 
		long int m_MsInStatus[NUM_STATES];
		int m_NextStatusIfExceededMaxMsInStatus[NUM_STATES];    // Next State to go if exceeded the maximum time in the state
		int m_actualStatus;
		int m_oldStatus;
		void* m_pStructData;
		bool m_bLogEnable;
		int m_msecCycle;
		E_STATE_MACHINE_ERROR m_StateError;
		int m_StatusInErr;
	public:
	
		CStateMachine()
		{
			m_StateError = NO_ERROR;
			m_StatusInErr = 0;
			m_actualStatus = 0;
			m_oldStatus = 0;
			m_bLogEnable = false;
			for (int i = 0; i < NUM_STATES; i++)
			{
				m_MaxMsInStatus[i] = 0;
				m_fChangeStatus[i] = nullptr;
                for (int j = 0; j < NUM_STATES; j++)
                {
                    m_fTransition[i][j] = nullptr;
                }
			}
		};
		virtual ~CStateMachine() {};

//
// AssignData
//
		void AssignData(void* pstruct_data = nullptr, int msecCycle = 0)
		{
			if (m_pStructData == nullptr)
			{
				//throw invalid_argument("Undefined data"); 
			}
			m_pStructData = pstruct_data;
			m_msecCycle = msecCycle;
		};
//
// AssignState
//
		void AssignState(int ind,
						myStatusFunc            fStatus = nullptr,
						myDropOutFunc           fDropOut = nullptr,
						myTransitionFunc*       fTransition = nullptr,
						myPickUpFunc            fPickUp = nullptr,
						myChangeStatusFunc      fChangeStatusFunc = nullptr,
						long int                MaxMsInStatus = 0,
						int                     NextStatusIfOverMaxMsInStatus = 0,
						const char*             stausName = nullptr) 
		{
			char txt[512];
            
			if (m_bLogEnable)
			{
				sprintf(txt,"\n*** StateMachine::AssignState\n ind:%d\n fStatus:%p\n fDropOut:%p\n fTransition:%p\n fPickUp:%p\n fChangeStatusFunc:%p\n MaxMsInStatus:%d\n NextStatusIfOverMaxMsInStatus:%d\n stausName%d",
						ind,
						fStatus,
						fDropOut,
						fTransition,
						fPickUp,
						fChangeStatusFunc,
						MaxMsInStatus,
						NextStatusIfOverMaxMsInStatus,
						stausName);
				Serial.println(txt);
				Serial.println("-    -    -");
			}
			if (ind < NUM_STATES)
			{   
				// Check Parameters
                if (MaxMsInStatus < 0)
                {
                    m_StateError = NO_CHANGE_STATE_DEFINED;
                    m_StatusInErr = ind;
                }
				if (fChangeStatusFunc == nullptr)
				{
					if ((MaxMsInStatus == 0) || (NextStatusIfOverMaxMsInStatus == ind))
					{
						m_StateError = NO_CHANGE_STATE_DEFINED;
						m_StatusInErr = ind;
					}
				}
				if (m_fChangeStatus[ind] != nullptr)
				{
					m_StateError = ALREADY_DEFINED_STATE;
					m_StatusInErr = ind;
				}
				if ((MaxMsInStatus > 0) && (m_msecCycle == 0))
				{
					m_StateError = UNDFINED_CYCLE_TIME;
					m_StatusInErr = ind;
				}
				if ((MaxMsInStatus > 0) && (NextStatusIfOverMaxMsInStatus == ind))
				{
					m_StateError = WRONG_IND_STATE_MAX_IN_TRANS;
					m_StatusInErr = ind;
				}
				if (NextStatusIfOverMaxMsInStatus >= NUM_STATES)
				{
					m_StateError = IND_STATE_OVER_MAX_IN_TRANS;
					m_StatusInErr = ind;
				}
				if (m_StateError != NO_ERROR)
				{
					return;
				}
				// Parameters are OK
				m_fStatus[ind] = fStatus;
				m_fDropOut[ind] = fDropOut;
                if (fTransition != nullptr)
                {
                    for (int i = 0; i < NUM_STATES; i++)
                    {
                        m_fTransition[ind][i] = fTransition[i];
                    }
                }
				m_fPickUp[ind] = fPickUp;
				m_fChangeStatus[ind] = fChangeStatusFunc;
				m_MaxMsInStatus[ind] = MaxMsInStatus;
				m_NextStatusIfExceededMaxMsInStatus[ind] = NextStatusIfOverMaxMsInStatus;
				m_StatusName [ind] = stausName;
				if (m_bLogEnable)
				{
					sprintf(txt,"StateMachine::AssignState - ind:%d Assigned\n fStatus:%p\n fDropOut:%p\n fTransition:---\n fPickUp:%p\n fChangeStatusFunc:%p\n stausName:%s\n ---",
							ind,
							m_fStatus[ind],
							m_fDropOut[ind],
							m_fPickUp[ind],
							m_fChangeStatus[ind],
							stausName);
					Serial.println(txt);
				}
			delay (500);
			}
			else
			{
				m_StateError = IND_STATE_OVER_MAX;
				m_StatusInErr = ind;
			}
		};
//
// Manage 
//
		void Manage()
		{
			char txt[124];
			if (m_bLogEnable)
			{
				sprintf(txt,"StateMachine::Manage - State: %s [%d] -------- ", m_StatusName[m_actualStatus],m_actualStatus);
				Serial.println(txt);
			}
			if (m_StateError != NO_ERROR)
			{
				if (m_bLogEnable)
				{
					sprintf(txt,"StateMachine::Manage - Status:%d - Init Data Error:%d-%s Exit!", m_StatusInErr, m_StateError,stateMachineErrorString[m_StateError]);
					Serial.println(txt);
				}
				return;
			}
			bool bMaxMsInStatus = false;
			
			// Things to do in actual state
			if (*m_fStatus[m_actualStatus] != nullptr)
			{
				if (m_bLogEnable)
				{
					sprintf(txt,"StateMachine::Manage - Call State function");
					Serial.println(txt);
				}
				(*m_fStatus[m_actualStatus])(m_pStructData);
			}
			
			// Check the stay time in the state
			// if exceeded the maximum, perform a state change
			// otherwise
			// call the chageStatus function
			if (m_MaxMsInStatus[m_actualStatus] > 0)
			{
				m_MsInStatus[m_actualStatus] += m_msecCycle;
				if (m_bLogEnable)
				{
					sprintf(txt,"StateMachine::Manage - MaxMsInStatus:%d Check the stay time in the state, now is:%d",
								m_MaxMsInStatus[m_actualStatus],
								m_MsInStatus[m_actualStatus]);
					Serial.println(txt);
				}
				bMaxMsInStatus = m_MsInStatus[m_actualStatus] >= m_MaxMsInStatus[m_actualStatus];
			}
			if (bMaxMsInStatus)
			{
				m_MsInStatus[m_actualStatus] = 0;
				m_actualStatus = m_NextStatusIfExceededMaxMsInStatus[m_actualStatus];
				if (m_bLogEnable)
				{
					sprintf(txt,"StateMachine::Manage - Exceeded the maximum, perform a state change to:%s [%d]", m_StatusName[m_actualStatus], m_actualStatus);
					Serial.println(txt);
				}
			}
			else
			{
				if (m_fChangeStatus[m_actualStatus] != nullptr)
				{
					m_actualStatus = (*m_fChangeStatus[m_actualStatus])(m_pStructData);
                    if (m_bLogEnable)
                    {
                        sprintf(txt,"StateMachine::Manage - Called ChangeState function go in:%d",m_actualStatus);
                        Serial.println(txt);
                    }
				}
			}
			
			// if the state is changed call the defined functions
			if (m_actualStatus != m_oldStatus)
			{
				if (m_bLogEnable)
				{
					char txt[64];
					sprintf(txt, "StateMachine::Manage - Change from %s to %s", m_StatusName[m_oldStatus], m_StatusName[m_actualStatus]);
					Serial.println(txt);
				}
				if (*m_fDropOut[m_oldStatus] != nullptr)
				{
					if (m_bLogEnable)
					{
						sprintf(txt,"StateMachine::Manage - Call DropOut function");
						Serial.println(txt);
					}
					(*m_fDropOut[m_oldStatus])(m_pStructData);
				}
				if (*m_fTransition[m_oldStatus][m_actualStatus] != nullptr)
				{
					if (m_bLogEnable)
					{
						sprintf(txt,"StateMachine::Manage - Call Transition function from%d to %d",m_oldStatus,m_actualStatus);
						Serial.println(txt);
					}
					(*m_fTransition[m_oldStatus][m_actualStatus])(m_pStructData);
				}
				m_oldStatus = m_actualStatus;
				if (*m_fPickUp[m_oldStatus] != nullptr)
				{
					if (m_bLogEnable)
					{
						sprintf(txt,"StateMachine::Manage - Call PickUp function");
						Serial.println(txt);
					}
					(*m_fPickUp[m_oldStatus])(m_pStructData);
				}
			}
		};
		
		// call GetInitError after all AssingState calling to check consintecy of init value
		E_STATE_MACHINE_ERROR GetInitError ()   { return m_StateError; };
		
		const char* GetInitErrorString() { return stateMachineErrorString[m_StateError].s; };

		int GetStatusInd() { return m_actualStatus; };
		const char* GetStatusName () { return m_StatusName[m_actualStatus]; };
		void EnableLog (bool b_enable = false) { m_bLogEnable = b_enable; };
//
// ShowStateData 
//
		void ShowStateData ()
		{
			char txt[64];
			Serial.println(" ----- StateMachine::ShowStateData ------");
			sprintf(txt,"pStructData: %s",m_pStructData!=nullptr?"defined":"-");
			Serial.println(txt);
			for (int i = 0; i < NUM_STATES; i++)
			{
				sprintf(txt,"----- ind:%d - %s -----", i, m_StatusName[i]);
				Serial.println(txt);
				sprintf(txt,"fStatus:  %s",m_fStatus[i]!=nullptr?"defined":"-");
				Serial.println(txt);
				sprintf(txt,"fDropOut: %s",m_fDropOut[i]!=nullptr?"defined":"-");
				Serial.println(txt);
				for (int j = 0; j < NUM_STATES; j++)
				{
					sprintf(txt,"fTransition[%d]: %s",j,m_fTransition[i][j]!=nullptr?"defined":"-");
					Serial.println(txt);
				}
				sprintf(txt,"fPickUp: %s",m_fPickUp[i]!=nullptr?"defined":"-");
				Serial.println(txt);
				sprintf(txt,"fChangeStatusFunc: %s",m_fChangeStatus[i]!=nullptr?"defined":"-");
				Serial.println(txt);
				sprintf(txt,"MaxMsInStatus: %d",m_MaxMsInStatus[i]);
				Serial.println(txt);
				sprintf(txt,"NextStatusIfExceededMaxMsInStatus: %d",m_NextStatusIfExceededMaxMsInStatus[i]);
				Serial.println(txt);
			}
		}
};

