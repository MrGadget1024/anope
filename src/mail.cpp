#include "services.h"

MailThread::~MailThread()
{
	if (Success)
		Log() << "Successfully delivered mail for " << MailTo << " (" << Addr << ")";
	else
		Log() << "Error delivering mail for " << MailTo << " (" << Addr << ")";
}

void MailThread::Run()
{
	FILE *pipe = popen(Config->SendMailPath.c_str(), "w");

	if (!pipe)
	{
		SetExitState();
		return;
	}

	fprintf(pipe, "From: %s\n", Config->SendFrom.c_str());
	if (Config->DontQuoteAddresses)
		fprintf(pipe, "To: %s <%s>\n", MailTo.c_str(), Addr.c_str());
	else
		fprintf(pipe, "To: \"%s\" <%s>\n", MailTo.c_str(), Addr.c_str());
	fprintf(pipe, "Subject: %s\n", Subject.c_str());
	fprintf(pipe, "%s", Message.c_str());
	fprintf(pipe, "\n.\n");

	pclose(pipe);

	Success = true;
	SetExitState();
}

bool Mail(User *u, NickRequest *nr, BotInfo *service, const Anope::string &subject, const Anope::string &message)
{
	if (!u || !nr || !service || subject.empty() || message.empty())
		return false;

	if (!Config->UseMail)
		u->SendMessage(service, MAIL_DISABLED);
	else if (Anope::CurTime - u->lastmail < Config->MailDelay)
		u->SendMessage(service, MAIL_DELAYED, Config->MailDelay - Anope::CurTime - u->lastmail);
	else if (nr->email.empty())
		u->SendMessage(service, MAIL_INVALID, nr->nick.c_str());
	else
	{
		u->lastmail = nr->lastmail = Anope::CurTime;
		threadEngine.Start(new MailThread(nr->nick, nr->email, subject, message));
		return true;
	}

	return false;
}

bool Mail(User *u, NickCore *nc, BotInfo *service, const Anope::string &subject, const Anope::string &message)
{
	if (!u || !nc || !service || subject.empty() || message.empty())
		return false;

	if (!Config->UseMail)
		u->SendMessage(service, MAIL_DISABLED);
	else if (Anope::CurTime - u->lastmail < Config->MailDelay)
		u->SendMessage(service, MAIL_DELAYED, Config->MailDelay - Anope::CurTime - u->lastmail);
	else if (nc->email.empty())
		u->SendMessage(service, MAIL_INVALID, nc->display.c_str());
	else
	{
		u->lastmail = nc->lastmail = Anope::CurTime;
		threadEngine.Start(new MailThread(nc->display, nc->email, subject, message));
		return true;
	}

	return false;
}

bool Mail(NickCore *nc, const Anope::string &subject, const Anope::string &message)
{
	if (!Config->UseMail || !nc || nc->email.empty() || subject.empty() || message.empty())
		return false;

	nc->lastmail = Anope::CurTime;
	threadEngine.Start(new MailThread(nc->display, nc->email, subject, message));

	return true;
}

/**
 * Checks whether we have a valid, common e-mail address.
 * This is NOT entirely RFC compliant, and won't be so, because I said
 * *common* cases. ;) It is very unlikely that e-mail addresses that
 * are really being used will fail the check.
 *
 * @param email Email to Validate
 * @return bool
 */
bool MailValidate(const Anope::string &email)
{
	bool has_period = false;

	static char specials[] = {'(', ')', '<', '>', '@', ',', ';', ':', '\\', '\"', '[', ']', ' '};

	if (email.empty())
		return false;
	Anope::string copy = email;

	size_t at = copy.find('@');
	if (at == Anope::string::npos)
		return false;
	Anope::string domain = copy.substr(at + 1);
	copy = copy.substr(0, at);

	/* Don't accept empty copy or domain. */
	if (copy.empty() || domain.empty())
		return false;

	/* Check for forbidden characters in the name */
	for (unsigned i = 0, end = copy.length(); i < end; ++i)
	{
		if (copy[i] <= 31 || copy[i] >= 127)
			return false;
		for (unsigned int j = 0; j < 13; ++j)
			if (copy[i] == specials[j])
				return false;
	}

	/* Check for forbidden characters in the domain */
	for (unsigned i = 0, end = domain.length(); i < end; ++i)
	{
		if (domain[i] <= 31 || domain[i] >= 127)
			return false;
		for (unsigned int j = 0; j < 13; ++j)
			if (domain[i] == specials[j])
				return false;
		if (domain[i] == '.')
		{
			if (!i || i == end - 1)
				return false;
			has_period = true;
		}
	}

	return has_period;
}