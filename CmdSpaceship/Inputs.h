#pragma once
#include <queue>
#include <optional>

class InputManager
{
public:

    // Helper function to send to network thread.
    std::optional<char> GetPendingGameInputToSend();
    void UpdatePendingSendGameInputQueue();

    // Helper function to send to game to handle (Local)
    void ReceiveRemoteGameInput(char key); // push
    void ReceiveLocalGameInput(char key); // push
    std::optional<char> GetLocalPendingInput(); // get
    std::optional<char> GetRemotePendingInput(); // get
    void UpdateLocalInputQueue(); // pop
    void UpdateRemoteInputQueue(); // pop

    // Diplay Bullet Fire Position from Another Person perspective
    void ReceiveRemoteCoordinate(const char* data);
    std::optional<const char*> GetCoordBuffer();
    void UpdateCoordBufferQueue();

    bool bHasWinner = false;
    bool bLoseFlag = false;

private:

    std::queue<char> m_PendingGameInputToSend;
    std::queue<char> m_PendingLocalGameInput;
    std::queue<char> m_PendingRemoteGameInput;

    std::queue<const char*> m_coordinateBuffer;
};

