/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiProcess.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 18:50:48 by tcali             #+#    #+#             */
/*   Updated: 2026/09/02 12:33:07 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CgiProcess.hpp"
#include "Utils.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <new>
#include <signal.h>
#include <sys/wait.h>
#include <cerrno>

CgiProcess::CgiProcess() : _pid(-1), _stdinFd(-1), _stdoutFd(-1), _exitStatus(-1),
	_inputOffset(0), _started(false), _stdinClosed(false), _stdoutClosed(false),
	_finished(false), _processExited(false), _exitedNormally(false)
{}

CgiProcess::~CgiProcess()
{
	closeFd(_stdinFd);
	closeFd(_stdoutFd);

	if (_pid > 0)
		terminateChild();
}

pid_t	CgiProcess::getPid() const
{
	return (_pid);
}

int	CgiProcess::getStdinFd() const
{
	return (_stdinFd);
}

int	CgiProcess::getStdoutFd() const
{
	return (_stdoutFd);
}

const str&	CgiProcess::getOutput() const
{
	return (_output);
}

int	CgiProcess::getExitStatus() const
{
	return (_exitStatus);
}

std::time_t	CgiProcess::getStartTime() const
{
	return (_startTime);
}

bool	CgiProcess::isStarted() const
{
	return (_started);
}

bool	CgiProcess::isFinished() const
{
	return (_finished);
}

bool	CgiProcess::exitedNormally() const
{
	return (_exitedNormally);
}

bool	CgiProcess::start(const str& interpreter, const str& scriptPath,
	const envMap& environment, const str& body)
{
	if (_started)
		return (false);

	int stdinPipe[2] = {-1, -1};
	int stdoutPipe[2] = {-1, -1};

	if (pipe(stdinPipe) == -1)
		return (false);

	if (pipe(stdoutPipe) == -1)
	{
		closePipe(stdinPipe);
		return (false);
	}

	char **envp = NULL;

	try
	{
		envp = buildEnvp(environment);
	}
	catch (...)
	{
		closePipe(stdinPipe);
		closePipe(stdoutPipe);
		throw;
	}

	_pid = fork();

	if (_pid == -1)
	{
		freeEnvp(envp);
		closePipe(stdinPipe);
		closePipe(stdoutPipe);
		_pid = -1;
		return (false);
	}

	if (_pid == 0)
		handleChildProcess(interpreter, scriptPath, envp, stdinPipe, stdoutPipe);

	freeEnvp(envp);
	envp = NULL;

	if (!handleParentProcess(stdinPipe, stdoutPipe, body))
	{
		closePipe(stdinPipe);
		closePipe(stdoutPipe);

		terminateChild();
		resetProcessState();

		return (false);
	}

	return (true);
}

bool CgiProcess::checkProcessStatus()
{
	if (!_started || _pid <= 0 || _processExited)
		return (_processExited);

	int	status = 0;

	pid_t	result = waitpid(_pid, &status, WNOHANG);

	if (result == 0)
		return (false);

	if (result == -1)
	{
		if (errno == EINTR)
			return (false);

		return (false);
	}

	_processExited = true;

	if (WIFEXITED(status))
	{
		_exitedNormally = true;
		_exitStatus = WEXITSTATUS(status);
	}
	else
	{
		_exitedNormally = false;
		_exitStatus = -1;
	}

	_pid = -1;

	updateFinishedState();

	return (true);
}

bool CgiProcess::readOutput()
{
	if (_stdoutFd == -1 || _stdoutClosed)
		return (true);

	char	buffer[4096];

	ssize_t bytes = read(_stdoutFd, buffer, sizeof(buffer));

	if (bytes < 0)
	{
		closeOutput();
		return (false);
	}

	if (bytes == 0)
	{
		closeOutput();
		return (true);
	}

	_output.append(buffer, static_cast<std::size_t>(bytes));

	return (true);
}

bool	CgiProcess::writeInput()
{
	if (_stdinFd == -1 || _stdinClosed)
		return (true);

	if (_inputOffset >= _input.size())
	{
		closeFd(_stdinFd);
		_stdinClosed = true;
		return (true);
	}

	const char	*data = _input.data() + _inputOffset;
	std::size_t	remaining = _input.size() - _inputOffset;

	ssize_t bytes = write(_stdinFd, data, remaining);

	if (bytes <= 0)
	{
		closeInput();
		return (false);
	}

	_inputOffset += static_cast<std::size_t>(bytes);

	if (_inputOffset >= _input.size())
		closeInput();

	return (true);
}

void	CgiProcess::closeInput()
{
	closeFd(_stdinFd);
	_stdinClosed = true;
}

void	CgiProcess::closeOutput()
{
	closeFd(_stdoutFd);
	_stdoutClosed = true;
	updateFinishedState();
}

char	**CgiProcess::buildEnvp(const envMap& environment) const
{
	char **envp = new char*[environment.size() + 1];

	std::size_t i = 0;

	for (envMap::const_iterator it = environment.begin(); it != environment.end();
	++it)
	{
		str entry = it->first + "=" + it->second;

		envp[i] = new char[entry.size() + 1];

		std::strcpy(envp[i], entry.c_str());

		++i;
	}

	envp[i] = NULL;

	return (envp);
}

void	CgiProcess::freeEnvp(char **envp) const
{
	if (envp == NULL)
		return;

	for (std::size_t i = 0; envp[i] != NULL; ++i)
		delete [] envp[i];

	delete [] envp;
}

void	CgiProcess::handleChildProcess(const str& interpreter, const str& scriptPath,
	char **envp, int stdinPipe[2], int stdoutPipe[2])
{

	if (signal(SIGPIPE, SIG_DFL) == SIG_ERR)
		_exit(126);

	close(stdinPipe[1]);
	close(stdoutPipe[0]);

	if (dup2(stdinPipe[0], STDIN_FILENO) == -1)
		_exit(126);

	if (dup2(stdoutPipe[1], STDOUT_FILENO) == -1)
		_exit(126);

	close(stdinPipe[0]);
	close(stdoutPipe[1]);

	str	scriptDirectory = getScriptDirectory(scriptPath);
	str	scriptName = getScriptName(scriptPath);

	if (chdir(scriptDirectory.c_str()) == -1)
		_exit(126);

	char *argv[3];

	argv[0] = const_cast<char*>(interpreter.c_str());
	argv[1] = const_cast<char*>(scriptName.c_str());
	argv[2] = NULL;

	execve(interpreter.c_str(), argv, envp);

	_exit(127);
}

bool	CgiProcess::handleParentProcess(int stdinPipe[2], int stdoutPipe[2],
	const str& body)
{
	close(stdinPipe[0]);
	stdinPipe[0] = -1;

	close(stdoutPipe[1]);
	stdoutPipe[1] = -1;

	_stdinFd = stdinPipe[1];
	_stdoutFd = stdoutPipe[0];

	stdinPipe[1] = -1;
	stdoutPipe[0] = -1;

	try
	{
		if (!setNonBlocking(_stdinFd) || !setNonBlocking(_stdoutFd))
			return (false);
	}
	catch (...)
	{
		return (false);
	}

	_input = body;
	_output.clear();
	_inputOffset = 0;

	_started = true;
	_stdinClosed = false;
	_stdoutClosed = false;
	_processExited = false;
	_finished = false;
	_startTime = std::time(NULL);

	if (_input.empty())
	{
		closeFd(_stdinFd);
		_stdinClosed = true;
	}
	
	return (true);
}

void	CgiProcess::closeFd(int& fd)
{
	if (fd != -1)
	{
		close(fd);
		fd = -1;
	}
}

void CgiProcess::closePipe(int pipeFd[2])
{
	if (pipeFd[0] != -1)
	{
		close(pipeFd[0]);
		pipeFd[0] = -1;
	}

	if (pipeFd[1] != -1)
	{
		close(pipeFd[1]);
		pipeFd[1] = -1;
	}
}

void CgiProcess::terminateChild()
{
	if (_pid <= 0)
		return;

	kill(_pid, SIGKILL);

	while (waitpid(_pid, NULL, 0) == -1)
	{
		if (errno != EINTR)
			break;
	}

	_pid = -1;
}

void CgiProcess::resetProcessState()
{
	closeFd(_stdinFd);
	closeFd(_stdoutFd);

	_input.clear();
	_output.clear();

	_inputOffset = 0;

	_started = false;
	_stdinClosed = false;
	_stdoutClosed = false;
	_processExited = false;
	_finished = false;
	_exitedNormally = false;
	_exitStatus = -1;
	_startTime = 0;
}

void CgiProcess::updateFinishedState()
{
	if (_processExited && _stdoutClosed)
		_finished = true;
}

str	CgiProcess::getScriptDirectory(
	const str& scriptPath) const
{
	str::size_type	pos = scriptPath.find_last_of('/');

	if (pos == str::npos)
		return (".");

	if (pos == 0)
		return ("/");

	return (scriptPath.substr(0, pos));
}

str	CgiProcess::getScriptName(
	const str& scriptPath) const
{
	str::size_type	pos = scriptPath.find_last_of('/');

	if (pos == str::npos)
		return (scriptPath);

	return (scriptPath.substr(pos + 1));
}
