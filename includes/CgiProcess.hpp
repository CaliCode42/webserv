/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiProcess.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tcali <tcali@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/18 18:50:17 by tcali             #+#    #+#             */
/*   Updated: 2026/08/18 20:51:46 by tcali            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIPROCESS_HPP
#define CGIPROCESS_HPP

#include <string>
#include <map>
#include <sys/types.h>

class CgiProcess
{
public:
	typedef std::map<std::string, std::string> envMap;

private:
	pid_t       _pid;

	int         _stdinFd;
	int         _stdoutFd;
	int			_exitStatus;


	std::string _input;
	std::string _output;

	std::size_t _inputOffset;

	bool        _started;
	bool        _stdinClosed;
	bool        _stdoutClosed;
	bool        _finished;
	bool		_processExited;
	bool		_exitedNormally;

	char	**buildEnvp(const envMap& environment) const;
	void	freeEnvp(char **envp) const;

	void	handleChildProcess(const std::string& interpreter,
		const std::string& scriptPath, char **envp, int stdinPipe[2],
		int stdoutPipe[2]);
	bool	handleParentProcess(int stdinPipe[2], int stdoutPipe[2],
		const std::string& body);
		
	void	closeFd(int& fd);
	void	closePipe(int pipeFd[2]);
	
	void	terminateChild();
	void	resetProcessState();
	void	updateFinishedState();

public:
	CgiProcess();
	~CgiProcess();

	pid_t				getPid() const;
	int					getStdinFd() const;
	int					getStdoutFd() const;

	const std::string&	getOutput() const;

	bool 				isStarted() const;
	bool 				isFinished() const;

	bool 				start(const std::string& interpreter,
		const std::string& scriptPath, const envMap& environment,
		const std::string& body);

	bool	checkProcessStatus();
	bool	readOutput();
};

#endif