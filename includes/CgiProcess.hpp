/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiProcess.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 18:50:17 by tcali             #+#    #+#             */
/*   Updated: 2026/08/24 19:43:56 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIPROCESS_HPP
#define CGIPROCESS_HPP

#include <string>
#include <map>
#include <sys/types.h>
#include <ctime>

typedef std::string str;

class CgiProcess
{
public:
	typedef std::map<str, str> envMap;
	
	CgiProcess();
	~CgiProcess();

	pid_t				getPid() const;
	int					getStdinFd() const;
	int					getStdoutFd() const;
	const str&			getOutput() const;
	int					getExitStatus() const;
	std::time_t			getStartTime() const;

	bool 				isStarted() const;
	bool 				isFinished() const;
	bool				exitedNormally() const;

	bool 				start(const str& interpreter, const str& scriptPath,
		const envMap& environment, const str& body);

	bool				checkProcessStatus();
	bool				readOutput();
	bool				writeInput();

	void				closeInput();
	void				closeOutput();

private:
	pid_t       _pid;

	int         _stdinFd;
	int         _stdoutFd;
	int			_exitStatus;
	std::time_t	_startTime;

	str 		_input;
	str 		_output;

	std::size_t _inputOffset;

	bool        _started;
	bool        _stdinClosed;
	bool        _stdoutClosed;
	bool        _finished;
	bool		_processExited;
	bool		_exitedNormally;

	char		**buildEnvp(const envMap& environment) const;
	void		freeEnvp(char **envp) const;

	void		handleChildProcess(const str& interpreter, const str& scriptPath,
		char **envp, int stdinPipe[2], int stdoutPipe[2]);
	bool		handleParentProcess(int stdinPipe[2], int stdoutPipe[2],
		const str& body);
		
	void		closeFd(int& fd);
	void		closePipe(int pipeFd[2]);
	
	void		terminateChild();
	void		resetProcessState();
	void		updateFinishedState();

	str			getScriptDirectory(const str& scriptPath) const;
	str			getScriptName(const str& scriptPath) const;
};

#endif